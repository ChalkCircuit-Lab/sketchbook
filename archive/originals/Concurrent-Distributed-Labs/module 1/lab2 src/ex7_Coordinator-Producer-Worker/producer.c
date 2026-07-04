#include <time.h>
#include "common.h"

// Submit one task to the coordinator. Returns 1 on success, 0 on failure.
int submit_task(int id, int duration, const char* desc) {
    int sock = connect_to_coordinator();
    if (sock < 0) {
        printf("[Producer] Connection failed.\n");
        return 0;
    }

    Message msg;
    msg.type = MSG_SUBMIT;
    msg.task_id = id;
    msg.duration = duration;
    strncpy(msg.description, desc, DESC_SIZE - 1);
    msg.description[DESC_SIZE - 1] = '\0';

    send(sock, &msg, sizeof(msg), 0);

    char ack[4] = {0};
    recv(sock, ack, 3, 0);
    close(sock);

    if (strcmp(ack, "ACK") == 0) {
        printf("[Producer] Task %d accepted: '%s' (%ds)\n", id, desc, duration);
        return 1;
    }
    printf("[Producer] Task %d: no ACK received.\n", id);
    return 0;
}

int main(int argc, char* argv[]) {
    srand(time(NULL));

    // Mode 1: Interactive (read from stdin)
    // Usage: ./producer
    //   Then type: "task description" or just press Enter for auto-generated tasks
    //   Type "quit" to exit.
    //
    // Mode 2: Auto-generate N tasks
    // Usage: ./producer <count>

    if (argc > 1) {
        // Auto mode: generate N tasks
        int count = atoi(argv[1]);
        if (count <= 0) count = 5;

        printf("[Producer] Auto-generating %d tasks...\n", count);
        for (int i = 1; i <= count; i++) {
            char desc[DESC_SIZE];
            snprintf(desc, DESC_SIZE, "Auto-Task-%d", i);
            int duration = 2 + rand() % 6; // 2-7 seconds
            submit_task(i, duration, desc);
        }
    } else {
        // Interactive mode
        printf("[Producer] Interactive mode. Type a task description (or 'quit'):\n");
        int id = 1;
        char line[256];

        while (1) {
            printf("> ");
            fflush(stdout);

            if (!fgets(line, sizeof(line), stdin)) break;

            // Strip newline
            line[strcspn(line, "\n")] = '\0';

            if (strcmp(line, "quit") == 0 || strcmp(line, "q") == 0) break;

            const char* desc = (strlen(line) > 0) ? line : "Default Task";
            int duration = 2 + rand() % 6;
            submit_task(id++, duration, desc);
        }
    }

    printf("[Producer] Done.\n");
    return 0;
}
