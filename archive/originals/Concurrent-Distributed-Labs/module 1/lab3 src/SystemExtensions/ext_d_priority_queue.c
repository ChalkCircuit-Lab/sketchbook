// =============================================================================
// EXTENSION D: SMART SCHEDULING (Priority Queue)
// =============================================================================
//
// WHAT THIS FIXES vs. the original lab doc:
//   1. BREAKING CHANGE WARNING: Adding "int priority" to Message changes its
//      size (76 -> 80 bytes). ALL components that send/receive Message must be
//      updated simultaneously, or recv_all will hang waiting for bytes that
//      never arrive. Affected files listed below.
//   2. dequeue() simplified — no need to shift after dequeue if we always
//      insert in sorted order and dequeue from index 0. The original doc's
//      dequeue was correct but the shift can be simplified.
//   3. client.c updated to accept priority as a command-line argument.
//   4. exploit.py struct format updated from "iii" to "iiii".
//
// HOW TO PLUG IN:
//   - Update common.h (struct change)
//   - Replace enqueue/dequeue in coordinator_secure.c
//   - Update client.c to set priority
//   - Update exploit.py struct format
//   - Update dos_flooder.c to set msg.priority
//
// IMPORTANT: Recompile ALL components after making the struct change!
// =============================================================================

// ---- common.h: Replace the Message struct ----

/*
typedef struct {
    int type;           // 1 = Client, 2 = Worker, 3 = Task Done
    int id;             // Task ID
    int duration;       // Simulated processing time
    int priority;       // NEW: 1 (Low) to 10 (Critical)
    char description[DESC_SIZE];
} Message;
*/

// ---- coordinator_secure.c: Replace the TaskQueue and enqueue/dequeue ----
// NOTE: With priority scheduling, we switch from a ring buffer to a
// sorted linear array. The head/tail pointers are no longer needed.

/*
typedef struct {
    Message tasks[TASK_QUEUE_SIZE];
    int count;              // No head/tail — always insert sorted, dequeue from 0
    pthread_mutex_t lock;
} TaskQueue;

TaskQueue task_queue;

// Sorted insert: higher priority values go toward index 0.
// O(N) insertion — acceptable for TASK_QUEUE_SIZE = 100.
void enqueue(Message t) {
    pthread_mutex_lock(&task_queue.lock);

    if (task_queue.count < TASK_QUEUE_SIZE) {
        // Find insertion point: scan backwards while existing items have lower priority
        int i;
        for (i = task_queue.count - 1; i >= 0; i--) {
            if (task_queue.tasks[i].priority >= t.priority) break;
            task_queue.tasks[i + 1] = task_queue.tasks[i]; // Shift right
        }
        task_queue.tasks[i + 1] = t;
        task_queue.count++;
        printf("[Queue] Task %d (Priority %d) inserted. Total: %d\n",
               t.id, t.priority, task_queue.count);
    } else {
        printf("[Queue] FULL! Task %d (Priority %d) dropped.\n", t.id, t.priority);
    }

    pthread_mutex_unlock(&task_queue.lock);
}

// Always dequeue from index 0 (highest priority). Shift remaining left.
// O(N) dequeue — acceptable for small queue sizes.
int dequeue(Message *t) {
    pthread_mutex_lock(&task_queue.lock);
    int success = 0;

    if (task_queue.count > 0) {
        *t = task_queue.tasks[0];

        // Shift everything left
        for (int i = 0; i < task_queue.count - 1; i++) {
            task_queue.tasks[i] = task_queue.tasks[i + 1];
        }
        task_queue.count--;
        success = 1;
    }

    pthread_mutex_unlock(&task_queue.lock);
    return success;
}
*/

// ---- coordinator_secure.c: Update main() init ----
// Replace: task_queue.head = 0; task_queue.tail = 0; task_queue.count = 0;
// With:    task_queue.count = 0;

// ---- client.c: Update to accept priority argument ----

/*
    // After msg.duration assignment, add:
    msg.priority = 5; // Default: medium priority
    if (argc > 2) {
        msg.priority = atoi(argv[2]);
        if (msg.priority < 1) msg.priority = 1;
        if (msg.priority > 10) msg.priority = 10;
    }

    // Update the printf to show priority:
    printf("[Client] Submitting Task ID %d (Priority %d): '%s' (Est. Time: %ds)\n",
           msg.id, msg.priority, msg.description, msg.duration);
*/
// Usage: ./client "My Task" 10    (priority 10 = critical)
//        ./client "My Task" 1     (priority 1 = low)
//        ./client                  (priority 5 = default)

// ---- exploit.py: Update struct format ----
// The Message struct is now: type(4) + id(4) + duration(4) + priority(4) + desc
// Change from:
//   header = struct.pack("iii", 1, 666, 0)
// To:
//   header = struct.pack("iiii", 1, 666, 0, 0)  # 16 bytes now

// ---- dos_flooder.c: Set priority field ----
// In flood_thread, after msg.duration = 9999, add:
//   msg.priority = 1;  // Low priority junk

// ---- worker_secure.c: No changes needed ----
// Workers don't read the priority field. They just execute whatever task
// the coordinator assigns. The coordinator handles priority ordering.

// ---- Testing ----
// 1. Recompile ALL components (coordinator, client, worker, flooder, exploit).
// 2. Start coordinator and worker.
// 3. Send: ./client "Batch Job" 1        (low priority, long task)
// 4. Send: ./client "Urgent Fix" 10      (high priority)
// 5. Send: ./client "Normal Task" 5      (medium priority)
// 6. Worker should pick up tasks in order: Urgent(10), Normal(5), Batch(1).
