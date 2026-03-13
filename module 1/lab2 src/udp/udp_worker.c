// =============================================================================
// UDP Worker
//
// Polls the coordinator for tasks via UDP.  When it receives a task it
// "processes" it (sleep), then sends a MSG_DONE datagram.
//
// Because UDP is unreliable, the DONE report may be lost.  A production
// system would add sequence numbers / retransmission -- this PoC keeps it
// simple for demonstration purposes.
//
// Usage: ./udp_worker [poll_interval_sec]    (default: 2s)
// =============================================================================

#include "udp_common.h"
#include <time.h>

#define RECV_TIMEOUT_SEC 3

int main(int argc, char *argv[]) {
    int poll_sec = (argc > 1) ? atoi(argv[1]) : 2;

    int sock = udp_socket();
    if (sock < 0) return 1;

    // Set receive timeout so recvfrom does not block forever
    struct timeval tv = { .tv_sec = RECV_TIMEOUT_SEC, .tv_usec = 0 };
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in dest = coordinator_addr();

    printf("[worker] Polling coordinator every %ds\n", poll_sec);

    for (;;) {
        // Send READY request
        Message req = { .type = MSG_READY };
        udp_send(sock, &req, &dest);

        // Wait for response
        Message resp;
        struct sockaddr_in from;
        ssize_t n = udp_recv(sock, &resp, &from);

        if (n < (ssize_t)sizeof(resp)) {
            // Timeout or short read -- try again later
            sleep(poll_sec);
            continue;
        }

        if (resp.type == MSG_NOTASK) {
            printf("[worker] No tasks available, sleeping %ds...\n", poll_sec);
            sleep(poll_sec);
            continue;
        }

        if (resp.type == MSG_TASK) {
            resp.description[DESC_SIZE - 1] = '\0';
            printf("[worker] Received task #%d \"%s\" (dur=%ds)\n",
                   resp.task_id, resp.description, resp.duration);

            // Simulate processing
            sleep(resp.duration);

            // Report completion
            Message done = { .type = MSG_DONE, .task_id = resp.task_id };
            udp_send(sock, &done, &dest);
            printf("[worker] Completed task #%d\n", resp.task_id);
            continue;      // immediately ask for more
        }

        printf("[worker] Unexpected message type %d\n", resp.type);
        sleep(poll_sec);
    }

    close(sock);
    return 0;
}
