// =============================================================================
// EXTENSION E: RESOURCE MANAGEMENT (Cluster Monitoring Dashboard)
// =============================================================================
//
// WHAT THIS FIXES vs. the original lab doc:
//   1. Workers identified by a random worker_id (not IP address). All localhost
//      workers share IP 127.0.0.1 — IP alone can't distinguish them.
//   2. No system("clear") — that wipes all useful coordinator log output.
//      Instead, uses a separator and compact format.
//   3. Stale timeout increased to account for task duration. The original 10s
//      timeout would deregister workers still processing long tasks.
//   4. queue.count read under lock (was a race condition).
//   5. Worker sends a random ID in the Message.id field when connecting,
//      which the coordinator uses for registry identification.
//
// HOW TO PLUG IN:
//   - Add the registry code to coordinator_secure.c
//   - Update handle_connection to register workers
//   - Start the dashboard thread in main()
//   - Update worker_secure.c to send a persistent worker_id
// =============================================================================

// ---- coordinator_secure.c: Add after includes ----

#include <time.h>

#define MAX_WORKERS 20
#define WORKER_STALE_TIMEOUT 30  // Seconds before a worker is considered gone

typedef struct {
    int worker_id;          // Unique ID sent by the worker
    char ip_address[INET6_ADDRSTRLEN];
    int current_task_id;    // -1 = Idle, >0 = Working on task
    time_t last_seen;
    int registered;         // 1 = Active slot, 0 = Empty
} WorkerInfo;

WorkerInfo worker_registry[MAX_WORKERS];
pthread_mutex_t reg_lock = PTHREAD_MUTEX_INITIALIZER;

void init_registry() {
    for (int i = 0; i < MAX_WORKERS; i++)
        worker_registry[i].registered = 0;
}

// Register or update a worker in the registry.
// worker_id: unique ID from the worker process
// ip: client IP string
// task_id: -1 if idle, or the assigned task ID
void registry_update(int worker_id, const char* ip, int task_id) {
    pthread_mutex_lock(&reg_lock);

    // 1. Try to find existing worker by ID
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (worker_registry[i].registered && worker_registry[i].worker_id == worker_id) {
            worker_registry[i].current_task_id = task_id;
            worker_registry[i].last_seen = time(NULL);
            pthread_mutex_unlock(&reg_lock);
            return;
        }
    }

    // 2. Register new worker in first empty slot
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (!worker_registry[i].registered) {
            worker_registry[i].worker_id = worker_id;
            strncpy(worker_registry[i].ip_address, ip, INET6_ADDRSTRLEN - 1);
            worker_registry[i].ip_address[INET6_ADDRSTRLEN - 1] = '\0';
            worker_registry[i].current_task_id = task_id;
            worker_registry[i].last_seen = time(NULL);
            worker_registry[i].registered = 1;
            printf("[Registry] New worker %d registered from %s\n", worker_id, ip);
            pthread_mutex_unlock(&reg_lock);
            return;
        }
    }

    pthread_mutex_unlock(&reg_lock);
    printf("[Registry] WARNING: Registry full, cannot register worker %d\n", worker_id);
}

// ---- coordinator_secure.c: The Dashboard Thread ----

void* dashboard_thread(void* arg) {
    while (1) {
        sleep(5);

        // Read queue count under lock
        pthread_mutex_lock(&task_queue.lock);
        int q_count = task_queue.count;
        pthread_mutex_unlock(&task_queue.lock);

        // Print dashboard
        printf("\n========== CLUSTER DASHBOARD ==========\n");
        printf("  Task Queue: %d / %d\n", q_count, TASK_QUEUE_SIZE);
        printf("  %-12s %-18s %-8s %-10s\n", "Worker ID", "IP Address", "Status", "Task");
        printf("  ----------------------------------------\n");

        pthread_mutex_lock(&reg_lock);
        time_t now = time(NULL);
        int active = 0;

        for (int i = 0; i < MAX_WORKERS; i++) {
            if (!worker_registry[i].registered) continue;

            // Deregister stale workers
            if (difftime(now, worker_registry[i].last_seen) > WORKER_STALE_TIMEOUT) {
                printf("  [Worker %d timed out — deregistered]\n", worker_registry[i].worker_id);
                worker_registry[i].registered = 0;
                continue;
            }

            const char* status = (worker_registry[i].current_task_id == -1) ? "IDLE" : "BUSY";
            printf("  %-12d %-18s %-8s",
                   worker_registry[i].worker_id,
                   worker_registry[i].ip_address,
                   status);
            if (worker_registry[i].current_task_id != -1)
                printf(" #%d", worker_registry[i].current_task_id);
            printf("\n");
            active++;
        }

        pthread_mutex_unlock(&reg_lock);
        printf("  ----------------------------------------\n");
        printf("  Active Workers: %d\n", active);
        printf("========================================\n\n");
    }
    return NULL;
}

// ---- coordinator_secure.c: Update handle_connection() ----
// When a worker connects (MSG_WORKER_READY), extract its IP and register it.
// The worker sends its unique ID in msg.id (see worker changes below).

// You'll need this helper (also used by Extension B):
/*
void get_client_ip_from_sock(int sock, char* buf, size_t buflen) {
    struct sockaddr_storage addr;
    socklen_t len = sizeof(addr);
    getpeername(sock, (struct sockaddr*)&addr, &len);
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in* v4 = (struct sockaddr_in*)&addr;
        inet_ntop(AF_INET, &v4->sin_addr, buf, buflen);
    } else {
        struct sockaddr_in6* v6 = (struct sockaddr_in6*)&addr;
        inet_ntop(AF_INET6, &v6->sin6_addr, buf, buflen);
    }
}
*/

// In handle_connection, inside the MSG_WORKER_READY branch, after authentication:

/*
        // Get worker's IP for the registry
        char worker_ip[INET6_ADDRSTRLEN];
        get_client_ip_from_sock(sock, worker_ip, sizeof(worker_ip));
        int worker_id = msg.id; // Worker sends its unique ID here

        TaskResponse resp;
        Message t;
        if (dequeue(&t)) {
            resp.has_task = 1;
            resp.task_id = t.id;
            resp.duration = t.duration;
            strncpy(resp.description, t.description, DESC_SIZE);
            printf("[Coord] Assigning Task %d to Worker %d.\n", t.id, worker_id);

            // Register worker as BUSY with this task
            registry_update(worker_id, worker_ip, t.id);
        } else {
            resp.has_task = 0;
            // Register worker as IDLE
            registry_update(worker_id, worker_ip, -1);
        }
        send(sock, &resp, sizeof(resp), 0);
*/

// ---- coordinator_secure.c: Update main() ----

/*
    init_registry();
    pthread_t dash_th;
    pthread_create(&dash_th, NULL, dashboard_thread, NULL);
*/

// ---- worker_secure.c: Send a unique worker ID ----
// At the top of main(), generate a random ID that persists across reconnections:

/*
    srand(time(NULL) ^ getpid()); // Seed with PID for uniqueness
    int my_worker_id = rand() % 90000 + 10000; // 5-digit random ID

    printf(">>> WORKER %d STARTED. Connecting to: %s:%s...\n",
           my_worker_id, SERVER_IP, SERVER_PORT);

    while(1) {
        int sock = connect_to_coordinator();
        if (sock < 0) { sleep(2); continue; }

        // Identify with our unique worker ID
        Message msg = { .type = MSG_WORKER_READY, .id = my_worker_id };
        send(sock, &msg, sizeof(msg), 0);

        // ... rest of auth + task handling unchanged ...
    }
*/

// ---- Testing ----
// 1. Start coordinator_secure.
// 2. Start 2-3 worker instances in separate terminals.
// 3. Submit several tasks with ./client.
// 4. Every 5 seconds, the dashboard prints:
//      ========== CLUSTER DASHBOARD ==========
//        Task Queue: 2 / 100
//        Worker ID    IP Address         Status   Task
//        ----------------------------------------
//        48271        127.0.0.1          BUSY     #3847
//        91034        127.0.0.1          IDLE
//        ----------------------------------------
//        Active Workers: 2
//      ========================================
