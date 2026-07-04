#ifndef QUICHE_COMMON_H
#define QUICHE_COMMON_H

// =============================================================================
// Shared definitions for the QUIC Coordinator-Producer-Worker system
// using Cloudflare's quiche library.
//
// KEY DIFFERENCES vs. the MsQuic version (../quic/):
//   - quiche is "bring your own I/O": you create a UDP socket yourself,
//     receive datagrams with recvfrom(), pass them to quiche_conn_recv(),
//     and send data produced by quiche_conn_send() via sendto().
//   - No callbacks -- everything is driven by an explicit event loop.
//   - Connection management is manual: the server parses QUIC headers
//     to demultiplex packets to the correct quiche_conn.
//
// SETUP:
//   Requires building quiche from source (Rust toolchain needed).
//   The Makefile handles this automatically -- just run `make`.
// =============================================================================

#include <quiche.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <inttypes.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <poll.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

// --- CONFIG ---
#define QUICHE_PORT         4568
#define QUICHE_PORT_STR     "4568"
#define SERVER_IP           "127.0.0.1"
#define QUICHE_IDLE_MS      5000
#define MAX_DATAGRAM_SIZE   1350
#define LOCAL_CONN_ID_LEN   16

// --- PROTOCOL (same as MsQuic / TCP / UDP versions) ---
#define MSG_SUBMIT  1   // Producer -> Coordinator: here's a task
#define MSG_READY   2   // Worker   -> Coordinator: give me work
#define MSG_DONE    3   // Worker   -> Coordinator: task finished

#define DESC_SIZE   64

typedef struct {
    int type;                   // MSG_SUBMIT, MSG_READY, or MSG_DONE
    int task_id;
    int duration;
    char description[DESC_SIZE];
} Message;

typedef struct {
    int has_task;
    int task_id;
    int duration;
    char description[DESC_SIZE];
} TaskResponse;

// --- ALPN ---
// QUIC requires Application-Layer Protocol Negotiation.
// We use a custom protocol name: "lab3-quiche".
// Wire format: length-prefixed string.
static const uint8_t QUICHE_ALPN[] = "\x0blab3-quiche";
#define QUICHE_ALPN_LEN (sizeof(QUICHE_ALPN) - 1)

// =============================================================================
// HELPERS
// =============================================================================

// Generate a random connection ID.
static inline int gen_cid(uint8_t *cid, size_t cid_len) {
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) { perror("open /dev/urandom"); return -1; }
    ssize_t n = read(fd, cid, cid_len);
    close(fd);
    return (n == (ssize_t)cid_len) ? 0 : -1;
}

// Make a socket non-blocking.
static inline int make_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0) return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Send all queued QUIC packets for a connection out through the UDP socket.
// This must be called after every quiche_conn_recv(), stream write, or timeout.
static inline void flush_egress(int sock, quiche_conn *conn) {
    static uint8_t out[MAX_DATAGRAM_SIZE];
    quiche_send_info send_info;

    while (1) {
        ssize_t written = quiche_conn_send(conn, out, sizeof(out), &send_info);

        if (written == QUICHE_ERR_DONE) break;

        if (written < 0) {
            fprintf(stderr, "quiche_conn_send failed: %zd\n", written);
            return;
        }

        ssize_t sent = sendto(sock, out, written, 0,
                              (struct sockaddr *)&send_info.to,
                              send_info.to_len);
        if (sent != written) {
            perror("sendto");
            return;
        }
    }
}

// Create and bind a non-blocking UDP socket.  Returns fd or -1.
static inline int create_udp_socket(const char *host, const char *port,
                                    struct sockaddr_storage *local_addr,
                                    socklen_t *local_addr_len) {
    struct addrinfo hints = {
        .ai_family   = PF_UNSPEC,
        .ai_socktype = SOCK_DGRAM,
        .ai_protocol = IPPROTO_UDP,
    };
    struct addrinfo *local;
    if (getaddrinfo(host, port, &hints, &local) != 0) {
        perror("getaddrinfo"); return -1;
    }

    int sock = socket(local->ai_family, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); freeaddrinfo(local); return -1; }

    if (bind(sock, local->ai_addr, local->ai_addrlen) < 0) {
        perror("bind"); close(sock); freeaddrinfo(local); return -1;
    }

    if (make_nonblocking(sock) < 0) {
        perror("fcntl nonblock"); close(sock); freeaddrinfo(local); return -1;
    }

    if (local_addr) {
        memcpy(local_addr, local->ai_addr, local->ai_addrlen);
        *local_addr_len = local->ai_addrlen;
    }

    freeaddrinfo(local);
    return sock;
}

// Create a quiche_config for the server (loads cert + key).
static inline quiche_config *create_server_config(const char *cert,
                                                   const char *key) {
    quiche_config *config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
    if (!config) return NULL;

    quiche_config_load_cert_chain_from_pem_file(config, cert);
    quiche_config_load_priv_key_from_pem_file(config, key);
    quiche_config_set_application_protos(config, QUICHE_ALPN, QUICHE_ALPN_LEN);

    quiche_config_set_max_idle_timeout(config, QUICHE_IDLE_MS);
    quiche_config_set_max_recv_udp_payload_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_max_send_udp_payload_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_initial_max_data(config, 10000000);
    quiche_config_set_initial_max_stream_data_bidi_local(config, 1000000);
    quiche_config_set_initial_max_stream_data_bidi_remote(config, 1000000);
    quiche_config_set_initial_max_streams_bidi(config, 100);
    quiche_config_set_cc_algorithm(config, QUICHE_CC_RENO);

    return config;
}

// Create a quiche_config for a client (no cert, skip server verification).
static inline quiche_config *create_client_config(void) {
    quiche_config *config = quiche_config_new(QUICHE_PROTOCOL_VERSION);
    if (!config) return NULL;

    quiche_config_set_application_protos(config, QUICHE_ALPN, QUICHE_ALPN_LEN);

    // Disable certificate verification (self-signed certs in the lab)
    quiche_config_verify_peer(config, false);

    quiche_config_set_max_idle_timeout(config, QUICHE_IDLE_MS);
    quiche_config_set_max_recv_udp_payload_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_max_send_udp_payload_size(config, MAX_DATAGRAM_SIZE);
    quiche_config_set_initial_max_data(config, 10000000);
    quiche_config_set_initial_max_stream_data_bidi_local(config, 1000000);
    quiche_config_set_initial_max_stream_data_bidi_remote(config, 1000000);
    quiche_config_set_initial_max_stream_data_uni(config, 1000000);
    quiche_config_set_initial_max_streams_bidi(config, 100);
    quiche_config_set_initial_max_streams_uni(config, 100);
    quiche_config_set_disable_active_migration(config, true);

    return config;
}

#endif // QUICHE_COMMON_H
