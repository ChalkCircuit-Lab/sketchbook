// =============================================================================
// UDP Coordinator
//
// Single-threaded event loop.  No threads, no accept() -- just one socket
// that handles messages from both producers and workers via recvfrom/sendto.
//
// Usage: ./udp_coordinator
// =============================================================================

#include "udp_common.h"

#define MAX_TASKS 256

// ---- Task Queue (simple circular buffer) ----
typedef struct {
    Message tasks[MAX_TASKS];
    int head, tail, count;
} TaskQueue;

static TaskQueue queue;
static int next_task_id = 1;

static void queue_init(TaskQueue *q) {
    q->head = q->tail = q->count = 0;
}

static int queue_full(TaskQueue *q) {
    return q->count >= MAX_TASKS;
}

static int queue_empty(TaskQueue *q) {
    return q->count == 0;
}

static void enqueue(TaskQueue *q, const Message *msg) {
    q->tasks[q->tail] = *msg;
    q->tail = (q->tail + 1) % MAX_TASKS;
    q->count++;
}

static Message dequeue(TaskQueue *q) {
    Message m = q->tasks[q->head];
    q->head = (q->head + 1) % MAX_TASKS;
    q->count--;
    return m;
}

// ---- Helpers ----
static const char *addr_str(struct sockaddr_in *a) {
    static char buf[64];
    snprintf(buf, sizeof(buf), "%s:%d",
             inet_ntoa(a->sin_addr), ntohs(a->sin_port));
    return buf;
}

// ---- Main Loop ----
int main(void) {
    int sock = udp_bind(SERVER_PORT);
    if (sock < 0) return 1;

    queue_init(&queue);
    printf("[coordinator] Listening on UDP port %d\n", SERVER_PORT);

    Message msg;
    struct sockaddr_in sender;

    for (;;) {
        ssize_t n = udp_recv(sock, &msg, &sender);
        if (n < 0) { perror("recvfrom"); continue; }
        if (n < (ssize_t)sizeof(msg)) {
            printf("[coordinator] Short datagram from %s (%zd bytes) -- ignored\n",
                   addr_str(&sender), n);
            continue;
        }

        switch (msg.type) {

        // ---- Producer submitted a task ----
        case MSG_SUBMIT: {
            msg.description[DESC_SIZE - 1] = '\0';      // safety
            if (queue_full(&queue)) {
                printf("[coordinator] Queue full, dropping task from %s\n",
                       addr_str(&sender));
                break;
            }
            msg.task_id = next_task_id++;
            enqueue(&queue, &msg);
            printf("[coordinator] Queued task #%d \"%s\" (dur=%ds) from %s  [queue=%d]\n",
                   msg.task_id, msg.description, msg.duration,
                   addr_str(&sender), queue.count);

            // ACK back to the producer
            Message ack = { .type = MSG_ACK, .task_id = msg.task_id };
            udp_send(sock, &ack, &sender);
            break;
        }

        // ---- Worker requests a task ----
        case MSG_READY: {
            if (queue_empty(&queue)) {
                Message no = { .type = MSG_NOTASK };
                udp_send(sock, &no, &sender);
                printf("[coordinator] No tasks for worker %s\n", addr_str(&sender));
            } else {
                Message task = dequeue(&queue);
                task.type = MSG_TASK;
                udp_send(sock, &task, &sender);
                printf("[coordinator] Dispatched task #%d to %s  [queue=%d]\n",
                       task.task_id, addr_str(&sender), queue.count);
            }
            break;
        }

        // ---- Worker reports completion ----
        case MSG_DONE: {
            printf("[coordinator] Worker %s completed task #%d\n",
                   addr_str(&sender), msg.task_id);
            break;
        }

        default:
            printf("[coordinator] Unknown message type %d from %s\n",
                   msg.type, addr_str(&sender));
        }
    }

    close(sock);
    return 0;
}
