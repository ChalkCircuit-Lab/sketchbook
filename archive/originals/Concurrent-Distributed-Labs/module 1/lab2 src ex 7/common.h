#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

// --- CONFIG ---
#define SERVER_PORT 9000
#define SERVER_IP   "127.0.0.1"

// --- PROTOCOL ---
#define MSG_SUBMIT  1   // Producer -> Coordinator: here's a task
#define MSG_READY   2   // Worker   -> Coordinator: give me work
#define MSG_DONE    3   // Worker   -> Coordinator: task finished

#define DESC_SIZE 64

// Sent by Producer (SUBMIT) and Worker (READY / DONE)
typedef struct {
    int type;                   // MSG_SUBMIT, MSG_READY, or MSG_DONE
    int task_id;
    int duration;               // Simulated processing time (seconds)
    char description[DESC_SIZE];
} Message;

// Sent by Coordinator in response to MSG_READY
typedef struct {
    int has_task;               // 1 = task assigned, 0 = queue empty
    int task_id;
    int duration;
    char description[DESC_SIZE];
} TaskResponse;

// --- HELPERS ---

// Read exactly 'len' bytes from a socket. Handles partial reads.
static inline ssize_t recv_all(int sock, void *buf, size_t len) {
    size_t total = 0;
    char *p = (char *)buf;
    while (total < len) {
        ssize_t n = recv(sock, p + total, len - total, 0);
        if (n <= 0) return n;
        total += n;
    }
    return total;
}

// Connect to the coordinator. Returns socket fd or -1 on failure.
static inline int connect_to_coordinator() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        close(sock);
        return -1;
    }
    return sock;
}

#endif
