#include "common.h"

int main() {
    printf("=== WORKER started ===\n");

    while (1) {
        // 1. Connect and request work
        int sock = connect_to_coordinator();
        if (sock < 0) {
            printf("[Worker] Cannot connect. Retrying in 2s...\n");
            sleep(2);
            continue;
        }

        Message msg = { .type = MSG_READY };
        send(sock, &msg, sizeof(msg), 0);

        // 2. Receive task assignment
        TaskResponse resp;
        if (recv_all(sock, &resp, sizeof(resp)) != sizeof(resp)) {
            close(sock);
            sleep(1);
            continue;
        }
        close(sock); // Free the connection while we work

        if (!resp.has_task) {
            printf("[Worker] No tasks available. Idle for 3s...\n");
            sleep(3);
            continue;
        }

        // 3. Process the task (simulated)
        printf("[Worker] Processing Task %d: '%s' (%ds)...\n",
               resp.task_id, resp.description, resp.duration);
        sleep(resp.duration);
        printf("[Worker] Task %d DONE.\n", resp.task_id);

        // 4. Report completion to coordinator
        int done_sock = connect_to_coordinator();
        if (done_sock >= 0) {
            Message done = { .type = MSG_DONE, .task_id = resp.task_id };
            send(done_sock, &done, sizeof(done), 0);
            close(done_sock);
        }
    }
    return 0;
}
