#include "common.h"
#define SECRET_KEY 0xCAFEBABE

int connect_to_coordinator() {
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    // Connect using IP address directly (no DNS resolution needed)
    if (getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &res) != 0) return -1;

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        close(sock); sock = -1;
    }
    freeaddrinfo(res);
    return sock;
}

int main() {
    printf(">>> WORKER STARTED. Connecting to: %s:%s...\n", SERVER_IP, SERVER_PORT);
    while(1) {
        int sock = connect_to_coordinator();
        if (sock < 0) {
            sleep(2); continue;
        }

        // 1. Identify
        Message msg = { .type = MSG_WORKER_READY };
        send(sock, &msg, sizeof(msg), 0);

        // 2. Auth Handshake
        unsigned int challenge;
        if (recv_all(sock, &challenge, sizeof(int)) > 0) {
            unsigned int response = challenge ^ SECRET_KEY;
            send(sock, &response, sizeof(int), 0);
        } else {
            close(sock); continue;
        }

        // 3. Get Work
        TaskResponse resp;
        if (recv_all(sock, &resp, sizeof(resp)) > 0 && resp.has_task) {
            printf("[Worker] Executing Task %d (%s)...\n", resp.task_id, resp.description);
            sleep(resp.duration); 
            printf("[Worker] Task %d DONE.\n", resp.task_id);
        } else {
            printf("[Worker] Idle.\n");
            sleep(3);
        }
        close(sock);
    }
    return 0;
}
