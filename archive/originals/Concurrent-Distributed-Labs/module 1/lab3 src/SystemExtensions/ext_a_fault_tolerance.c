// =============================================================================
// EXTENSION A: FAULT TOLERANCE (At-Least-Once Delivery)
// =============================================================================
//
// WHAT THIS FIXES vs. the original lab doc:
//   1. Added MSG_TASK_DONE message type so workers can report completion.
//      Without this, EVERY task would be retried after timeout (even successful ones).
//   2. Uses a separate pending_lock (not queue.lock) to avoid deadlock when
//      the monitor thread calls enqueue().
//   3. Worker is updated to send a completion notification after finishing.
//
// HOW TO PLUG IN:
//   - Add MSG_TASK_DONE to common.h
//   - Add the code blocks below to coordinator_secure.c (locations marked)
//   - Update worker_secure.c with the completion notification
// =============================================================================

// ---- common.h: Add this message code ----

#define MSG_TASK_DONE 3  // Worker reports task completion

// ---- coordinator_secure.c: Add after #define lines ----

#include <time.h>
#define TASK_TIMEOUT 15  // Seconds before a task is considered failed

typedef struct {
    Message task;
    time_t start_time;
    int active;       // 1 = Waiting for result, 0 = Empty slot
} PendingTask;

PendingTask pending_list[TASK_QUEUE_SIZE];
pthread_mutex_t pending_lock = PTHREAD_MUTEX_INITIALIZER; // Separate lock!

void init_pending() {
    for (int i = 0; i < TASK_QUEUE_SIZE; i++)
        pending_list[i].active = 0;
}

// Add a task to the pending list (called when assigning to a worker)
void pending_add(Message t) {
    pthread_mutex_lock(&pending_lock);
    for (int i = 0; i < TASK_QUEUE_SIZE; i++) {
        if (!pending_list[i].active) {
            pending_list[i].task = t;
            pending_list[i].start_time = time(NULL);
            pending_list[i].active = 1;
            break;
        }
    }
    pthread_mutex_unlock(&pending_lock);
}

// Remove a task from the pending list (called when worker reports DONE)
void pending_complete(int task_id) {
    pthread_mutex_lock(&pending_lock);
    for (int i = 0; i < TASK_QUEUE_SIZE; i++) {
        if (pending_list[i].active && pending_list[i].task.id == task_id) {
            pending_list[i].active = 0;
            printf("[Monitor] Task %d completed. Cleared from pending.\n", task_id);
            break;
        }
    }
    pthread_mutex_unlock(&pending_lock);
}

// ---- coordinator_secure.c: Add as a new thread function ----

// Background thread that checks for timed-out tasks and re-enqueues them.
// Uses pending_lock (NOT task_queue.lock) so it can safely call enqueue().
void* monitor_thread(void* arg) {
    while (1) {
        sleep(1);
        time_t now = time(NULL);

        pthread_mutex_lock(&pending_lock);
        for (int i = 0; i < TASK_QUEUE_SIZE; i++) {
            if (pending_list[i].active) {
                double elapsed = difftime(now, pending_list[i].start_time);

                if (elapsed > TASK_TIMEOUT) {
                    printf("[Monitor] Task %d timed out (%.0fs). Re-enqueuing...\n",
                           pending_list[i].task.id, elapsed);

                    Message retry_task = pending_list[i].task;
                    pending_list[i].active = 0;

                    // Safe: pending_lock != task_queue.lock, no deadlock
                    pthread_mutex_unlock(&pending_lock);
                    enqueue(retry_task);
                    pthread_mutex_lock(&pending_lock);
                }
            }
        }
        pthread_mutex_unlock(&pending_lock);
    }
    return NULL;
}

// ---- coordinator_secure.c: Update handle_connection() ----
// Replace the MSG_WORKER_READY block with this:

/*
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
            printf("[Coord] Assigning Task %d to Worker. Monitoring...\n", t.id);

            // NEW: Track this task in the pending list
            pending_add(t);

        } else {
            resp.has_task = 0;
        }
        send(sock, &resp, sizeof(resp), 0);
    }
    // NEW: Handle task completion reports from workers
    else if (msg.type == MSG_TASK_DONE) {
        printf("[Coord] Worker reports Task %d completed.\n", msg.id);
        pending_complete(msg.id);
    }
*/

// ---- coordinator_secure.c: Update main() ----
// Add before the thread pool creation:

/*
    init_pending();
    pthread_t mon_th;
    pthread_create(&mon_th, NULL, monitor_thread, NULL);
*/

// ---- worker_secure.c: Add completion notification ----
// Replace the task execution block with this:

/*
        // 3. Get Work
        TaskResponse resp;
        if (recv_all(sock, &resp, sizeof(resp)) > 0 && resp.has_task) {
            printf("[Worker] Executing Task %d (%s)...\n", resp.task_id, resp.description);
            close(sock); // Close the assignment connection

            sleep(resp.duration); // Simulate work
            printf("[Worker] Task %d DONE. Notifying coordinator...\n", resp.task_id);

            // NEW: Report completion back to coordinator
            int done_sock = connect_to_coordinator();
            if (done_sock >= 0) {
                Message done_msg = { .type = MSG_TASK_DONE, .id = resp.task_id };
                send(done_sock, &done_msg, sizeof(done_msg), 0);
                close(done_sock);
            }
        } else {
            printf("[Worker] Idle.\n");
            close(sock);
            sleep(3);
        }
*/
// Note: the close(sock) is moved before sleep(), and a new connection
// is opened to send MSG_TASK_DONE. This frees up the pool thread
// during the (potentially long) task execution.
