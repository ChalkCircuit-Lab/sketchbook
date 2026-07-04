#ifndef UDP_COMMON_H
#define UDP_COMMON_H

// =============================================================================
// Shared definitions for the UDP Coordinator-Producer-Worker system.
//
// KEY DIFFERENCES vs. TCP version:
//   - Connectionless: no connect/accept, just sendto/recvfrom
//   - Message-based: each sendto = exactly one datagram (clear boundaries)
//   - Unreliable: datagrams can be lost, duplicated, or arrive out of order
//   - No threads needed on coordinator: single socket, single event loop
//   - Max message size limited by UDP (typically ~65507 bytes, practically ~1400 for MTU)
// =============================================================================

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
#define MSG_ACK     4   // Coordinator -> Producer: task received
#define MSG_TASK    5   // Coordinator -> Worker:   here's your task
#define MSG_NOTASK  6   // Coordinator -> Worker:   queue is empty

#define DESC_SIZE 64

// Single message struct for ALL communication (fits in one UDP datagram).
// Unlike TCP, we don't need a separate TaskResponse struct --
// everything fits in one message since UDP preserves message boundaries.
typedef struct {
    int type;                   // MSG_SUBMIT, MSG_READY, MSG_DONE, MSG_ACK, MSG_TASK, MSG_NOTASK
    int task_id;
    int duration;
    char description[DESC_SIZE];
} Message;

// --- HELPERS ---

// Create and bind a UDP socket. Returns fd or -1.
static inline int udp_bind(int port) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(sock);
        return -1;
    }
    return sock;
}

// Create a UDP socket (no bind -- for clients). Returns fd or -1.
static inline int udp_socket() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) perror("socket");
    return sock;
}

// Fill a sockaddr_in for the coordinator.
static inline struct sockaddr_in coordinator_addr() {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &addr.sin_addr);
    return addr;
}

// Send a Message via UDP.
static inline ssize_t udp_send(int sock, const Message* msg,
                                struct sockaddr_in* dest) {
    return sendto(sock, msg, sizeof(*msg), 0,
                  (struct sockaddr *)dest, sizeof(*dest));
}

// Receive a Message via UDP. Fills sender address.
// Returns bytes received, or -1 on error.
static inline ssize_t udp_recv(int sock, Message* msg,
                                struct sockaddr_in* from) {
    socklen_t len = sizeof(*from);
    return recvfrom(sock, msg, sizeof(*msg), 0,
                    (struct sockaddr *)from, &len);
}

#endif
