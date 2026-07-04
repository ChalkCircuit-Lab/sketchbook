#include <pthread.h>
#include <sys/random.h> // Linux specific (Secure Random)
#include "common.h"

#define SECRET_KEY 0xCAFEBABE 
#define TASK_QUEUE_SIZE 100
#define THREAD_POOL_SIZE 50     // Fixed number of worker threads
#define CONN_QUEUE_SIZE  50     // Pending connection backlog

// --- TASK QUEUE (Ring Buffer) ---
typedef struct {
    Message tasks[TASK_QUEUE_SIZE];
    int head, tail, count;
    pthread_mutex_t lock;
} TaskQueue;

TaskQueue task_queue;

void enqueue(Message t) {
    pthread_mutex_lock(&task_queue.lock);
    if (task_queue.count < TASK_QUEUE_SIZE) {
        task_queue.tasks[task_queue.tail] = t;
        task_queue.tail = (task_queue.tail + 1) % TASK_QUEUE_SIZE;
        task_queue.count++;
        printf("[Coord] Task %d enqueued.\n", t.id);
    }
    pthread_mutex_unlock(&task_queue.lock);
}

int dequeue(Message *t) {
    pthread_mutex_lock(&task_queue.lock);
    int success = 0;
    if (task_queue.count > 0) {
        *t = task_queue.tasks[task_queue.head];
        task_queue.head = (task_queue.head + 1) % TASK_QUEUE_SIZE;
        task_queue.count--;
        success = 1;
    }
    pthread_mutex_unlock(&task_queue.lock);
    return success;
}

// --- CONNECTION QUEUE (for thread pool) ---
// Accepted sockets are placed here; worker threads pick them up.
typedef struct {
    int sockets[CONN_QUEUE_SIZE];
    int head, tail, count;
    pthread_mutex_t lock;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
} ConnQueue;

ConnQueue conn_queue;

void conn_queue_init() {
    conn_queue.head = 0;
    conn_queue.tail = 0;
    conn_queue.count = 0;
    pthread_mutex_init(&conn_queue.lock, NULL);
    pthread_cond_init(&conn_queue.not_empty, NULL);
    pthread_cond_init(&conn_queue.not_full, NULL);
}

// Called by accept loop — blocks if queue is full
void conn_enqueue(int sock) {
    pthread_mutex_lock(&conn_queue.lock);
    while (conn_queue.count >= CONN_QUEUE_SIZE) {
        // Queue full — accept loop stalls here until a worker finishes.
        // THIS is the DOS vulnerability: if all workers are stuck (slowloris),
        // this blocks forever and no new connections are served.
        pthread_cond_wait(&conn_queue.not_full, &conn_queue.lock);
    }
    conn_queue.sockets[conn_queue.tail] = sock;
    conn_queue.tail = (conn_queue.tail + 1) % CONN_QUEUE_SIZE;
    conn_queue.count++;
    pthread_cond_signal(&conn_queue.not_empty);
    pthread_mutex_unlock(&conn_queue.lock);
}

// Called by worker threads — blocks if queue is empty
int conn_dequeue() {
    pthread_mutex_lock(&conn_queue.lock);
    while (conn_queue.count == 0) {
        pthread_cond_wait(&conn_queue.not_empty, &conn_queue.lock);
    }
    int sock = conn_queue.sockets[conn_queue.head];
    conn_queue.head = (conn_queue.head + 1) % CONN_QUEUE_SIZE;
    conn_queue.count--;
    pthread_cond_signal(&conn_queue.not_full);
    pthread_mutex_unlock(&conn_queue.lock);
    return sock;
}

// --- SECURE AUTHENTICATION ---
int authenticate_worker(int sock) {
    unsigned int nonce;
    if (getrandom(&nonce, sizeof(nonce), 0) != sizeof(nonce)) return 0;

    send(sock, &nonce, sizeof(nonce), 0);

    unsigned int response;
    // Set a timeout for auth to prevent "Slowloris" attacks
    struct timeval tv = {2, 0}; 
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    
    if (recv_all(sock, &response, sizeof(response)) <= 0) return 0;

    return response == (nonce ^ SECRET_KEY);
}

// --- CONNECTION HANDLER (called by pool threads) ---
void handle_connection(int sock) {
    Message msg_buffer;
    
    // 1. ROBUST READ: Wait for exactly sizeof(Message) bytes
    //    NOTE: No timeout here — a slowloris attacker can stall this forever,
    //    tying up one pool thread per connection.
    if (recv_all(sock, &msg_buffer, sizeof(Message)) != sizeof(Message)) {
        close(sock); return;
    }

    // 2. SANITIZATION: enforce NULL termination
    Message msg = msg_buffer; 
    msg.description[DESC_SIZE - 1] = '\0'; 

    if (msg.type == MSG_TASK_SUBMIT) {
        printf("[Coord] Client submitted Task %d\n", msg.id);
        enqueue(msg);
        send(sock, "ACK", 3, 0);
    } 
    else if (msg.type == MSG_WORKER_READY) {
        if (!authenticate_worker(sock)) {
            printf("[Security] Auth Failed! Kicking connection.\n");
            close(sock); return;
        }

        TaskResponse resp;
        Message t;
        if (dequeue(&t)) {
            resp.has_task = 1;
            resp.task_id = t.id;
            resp.duration = t.duration;
            strncpy(resp.description, t.description, DESC_SIZE);
            printf("[Coord] Assigning Task %d to Worker.\n", t.id);
        } else {
            resp.has_task = 0;
        }
        send(sock, &resp, sizeof(resp), 0);
    }
    close(sock);
}

// --- THREAD POOL WORKER ---
void* pool_worker(void* arg) {
    int id = *(int*)arg;
    printf("[Pool] Worker thread %d ready.\n", id);
    while (1) {
        int sock = conn_dequeue();   // Block until a connection is available
        handle_connection(sock);     // Process it
        // Thread becomes available again for the next connection
    }
    return NULL;
}

int main() {
    task_queue.head = 0; task_queue.tail = 0; task_queue.count = 0;
    pthread_mutex_init(&task_queue.lock, NULL);
    conn_queue_init();

    int server_fd;
    struct addrinfo hints = {0}, *res;
    
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(NULL, SERVER_PORT, &hints, &res);
    server_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
    int opt = 1; 
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    bind(server_fd, res->ai_addr, res->ai_addrlen);
    listen(server_fd, 50);
    printf(">>> SECURE COORDINATOR listening on port %s (pool: %d threads)...\n",
           SERVER_PORT, THREAD_POOL_SIZE);

    // Create fixed-size thread pool
    pthread_t pool[THREAD_POOL_SIZE];
    int pool_ids[THREAD_POOL_SIZE];
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pool_ids[i] = i;
        pthread_create(&pool[i], NULL, pool_worker, &pool_ids[i]);
    }

    // Accept loop — hand off connections to the pool
    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);
        
        if (new_sock < 0) continue;

        conn_enqueue(new_sock); // Blocks if all pool threads are busy + queue is full
    }
    return 0;
}
