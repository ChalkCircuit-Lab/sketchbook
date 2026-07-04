#include <pthread.h>
#include "common.h"

#define QUEUE_SIZE 100

// --- Thread-safe Task Queue (Ring Buffer) ---
typedef struct {
    Message tasks[QUEUE_SIZE];
    int head, tail, count;
    pthread_mutex_t lock;
} TaskQueue;

TaskQueue queue;

void queue_init() {
    queue.head = queue.tail = queue.count = 0;
    pthread_mutex_init(&queue.lock, NULL);
}

void enqueue(Message t) {
    pthread_mutex_lock(&queue.lock);
    if (queue.count < QUEUE_SIZE) {
        queue.tasks[queue.tail] = t;
        queue.tail = (queue.tail + 1) % QUEUE_SIZE;
        queue.count++;
        printf("[Coord] Task %d enqueued (%s). Queue: %d\n",
               t.task_id, t.description, queue.count);
    } else {
        printf("[Coord] Queue FULL! Task %d dropped.\n", t.task_id);
    }
    pthread_mutex_unlock(&queue.lock);
}

int dequeue(Message *t) {
    pthread_mutex_lock(&queue.lock);
    int ok = 0;
    if (queue.count > 0) {
        *t = queue.tasks[queue.head];
        queue.head = (queue.head + 1) % QUEUE_SIZE;
        queue.count--;
        ok = 1;
    }
    pthread_mutex_unlock(&queue.lock);
    return ok;
}

// --- Connection Handler (one per client/worker connection) ---
void* handle_connection(void* arg) {
    int sock = *(int*)arg;
    free(arg);

    Message msg;
    if (recv_all(sock, &msg, sizeof(msg)) != sizeof(msg)) {
        close(sock);
        return NULL;
    }
    msg.description[DESC_SIZE - 1] = '\0'; // Sanitize

    switch (msg.type) {

    case MSG_SUBMIT: {
        printf("[Coord] Producer submitted Task %d: '%s' (%ds)\n",
               msg.task_id, msg.description, msg.duration);
        enqueue(msg);
        send(sock, "ACK", 3, 0);
        break;
    }

    case MSG_READY: {
        TaskResponse resp = {0};
        Message t;
        if (dequeue(&t)) {
            resp.has_task = 1;
            resp.task_id = t.task_id;
            resp.duration = t.duration;
            strncpy(resp.description, t.description, DESC_SIZE);
            printf("[Coord] Assigned Task %d to worker.\n", t.task_id);
        } else {
            resp.has_task = 0;
            printf("[Coord] No tasks. Worker goes idle.\n");
        }
        send(sock, &resp, sizeof(resp), 0);
        break;
    }

    case MSG_DONE: {
        printf("[Coord] Task %d completed by worker.\n", msg.task_id);
        break;
    }

    default:
        printf("[Coord] Unknown message type %d. Ignoring.\n", msg.type);
    }

    close(sock);
    return NULL;
}

// --- Main ---
int main() {
    queue_init();

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(SERVER_PORT);

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    if (listen(server_fd, 10) < 0) {
        perror("listen"); exit(1);
    }

    printf("=== COORDINATOR listening on port %d ===\n", SERVER_PORT);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t len = sizeof(client_addr);
        int *sock = malloc(sizeof(int));
        *sock = accept(server_fd, (struct sockaddr *)&client_addr, &len);

        if (*sock < 0) { free(sock); continue; }

        pthread_t th;
        pthread_create(&th, NULL, handle_connection, sock);
        pthread_detach(th);
    }
    return 0;
}
