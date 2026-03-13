// =============================================================================
// UDP Producer
//
// Sends tasks to the coordinator via UDP datagrams.
// Waits for an ACK after each submission (with a timeout to handle lost ACKs).
//
// Usage: ./udp_producer [count]        (default: 5 tasks)
// =============================================================================

#include "udp_common.h"
#include <time.h>

#define ACK_TIMEOUT_SEC 2   // seconds to wait for ACK before retransmitting

static const char *sample_descs[] = {
    "compress-log", "parse-csv", "resize-img",
    "encrypt-file", "run-report", "gen-thumbnail",
    "send-email",   "validate-xml", "sync-db",
};
#define N_DESCS (sizeof(sample_descs) / sizeof(sample_descs[0]))

int main(int argc, char *argv[]) {
    int count = (argc > 1) ? atoi(argv[1]) : 5;
    srand(time(NULL));

    int sock = udp_socket();
    if (sock < 0) return 1;

    // Set receive timeout so we can detect lost ACKs
    struct timeval tv = { .tv_sec = ACK_TIMEOUT_SEC, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in dest = coordinator_addr();

    printf("[producer] Submitting %d tasks to %s:%d via UDP\n",
           count, SERVER_IP, SERVER_PORT);

    for (int i = 0; i < count; i++) {
        Message msg;
        memset(&msg, 0, sizeof(msg));
        msg.type     = MSG_SUBMIT;
        msg.duration = 1 + rand() % 5;
        snprintf(msg.description, DESC_SIZE, "%s-%03d",
                 sample_descs[rand() % N_DESCS], i);

        // Retry loop: send, wait for ACK, retransmit on timeout
        int acked = 0;
        for (int attempt = 1; attempt <= 3 && !acked; attempt++) {
            udp_send(sock, &msg, &dest);

            Message ack;
            struct sockaddr_in from;
            ssize_t n = udp_recv(sock, &ack, &from);
            if (n == sizeof(ack) && ack.type == MSG_ACK) {
                printf("[producer] Task \"%s\" submitted (id=%d) [attempt %d]\n",
                       msg.description, ack.task_id, attempt);
                acked = 1;
            } else {
                printf("[producer] Timeout/bad ACK for \"%s\" (attempt %d/3)\n",
                       msg.description, attempt);
            }
        }

        if (!acked)
            printf("[producer] FAILED to submit \"%s\" after 3 attempts\n",
                   msg.description);
    }

    close(sock);
    printf("[producer] Done.\n");
    return 0;
}
