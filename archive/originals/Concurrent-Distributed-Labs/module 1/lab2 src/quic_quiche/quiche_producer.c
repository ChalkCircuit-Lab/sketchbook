// =============================================================================
// QUIC PRODUCER (Client) -- using Cloudflare quiche
//
// Submits tasks to the QUIC Coordinator.
// Equivalent to ../quic/quic_producer.c (MsQuic version).
//
// KEY CONCEPT: With quiche, the client manually drives the QUIC handshake
// by exchanging UDP datagrams in a loop until quiche_conn_is_established().
// Then it opens a bidirectional stream, sends a Message, and reads the
// response from the same stream.
//
// Usage: ./quiche_producer [count]   (default: interactive mode)
// =============================================================================

#include "quiche_common.h"

// --- Drive a connection: recv from socket -> feed to quiche -> flush egress ---
// Returns 0 on success, -1 if connection is closed.
static int drive_conn(int sock, quiche_conn *conn,
                      struct sockaddr_storage *local_addr,
                      socklen_t local_addr_len) {
    static uint8_t buf[65535];

    // Receive any pending datagrams
    while (1) {
        struct sockaddr_storage peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        memset(&peer_addr, 0, sizeof(peer_addr));

        ssize_t read_len = recvfrom(sock, buf, sizeof(buf), 0,
                                    (struct sockaddr *)&peer_addr,
                                    &peer_addr_len);
        if (read_len < 0) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) break;
            perror("recvfrom");
            return -1;
        }

        quiche_recv_info recv_info = {
            .from     = (struct sockaddr *)&peer_addr,
            .from_len = peer_addr_len,
            .to       = (struct sockaddr *)local_addr,
            .to_len   = local_addr_len,
        };

        ssize_t done = quiche_conn_recv(conn, buf, read_len, &recv_info);
        if (done < 0) {
            fprintf(stderr, "[producer] quiche_conn_recv: %zd\n", done);
            return -1;
        }
    }

    flush_egress(sock, conn);

    if (quiche_conn_is_closed(conn)) return -1;
    return 0;
}

// --- Connect to coordinator, complete handshake, return conn ---
// Returns quiche_conn* on success, NULL on failure.
// Caller must free with quiche_conn_free().
static quiche_conn *quic_connect(quiche_config *config, int sock,
                                  struct sockaddr_storage *local_addr,
                                  socklen_t local_addr_len,
                                  struct sockaddr *peer_addr,
                                  socklen_t peer_addr_len) {
    // Generate a random source connection ID
    uint8_t scid[LOCAL_CONN_ID_LEN];
    if (gen_cid(scid, sizeof(scid)) < 0) return NULL;

    quiche_conn *conn = quiche_connect(SERVER_IP,
                                        scid, sizeof(scid),
                                        (struct sockaddr *)local_addr,
                                        local_addr_len,
                                        peer_addr, peer_addr_len,
                                        config);
    if (!conn) {
        fprintf(stderr, "[producer] quiche_connect failed\n");
        return NULL;
    }

    // Send the initial handshake packets
    flush_egress(sock, conn);

    // Drive the handshake to completion
    struct pollfd pfd = { .fd = sock, .events = POLLIN };
    while (!quiche_conn_is_established(conn)) {
        int timeout = (int)quiche_conn_timeout_as_millis(conn);
        if (timeout < 0) timeout = 100;
        poll(&pfd, 1, timeout);

        quiche_conn_on_timeout(conn);

        if (drive_conn(sock, conn, local_addr, local_addr_len) < 0) {
            quiche_conn_free(conn);
            return NULL;
        }
    }

    return conn;
}

// --- Submit one task ---
static int submit_task(quiche_config *config,
                       struct sockaddr *peer_addr, socklen_t peer_addr_len,
                       int id, int duration, const char *desc) {
    // Create a fresh UDP socket for each task (simplest approach,
    // avoids state leakage between requests).
    struct sockaddr_storage local_addr;
    socklen_t local_addr_len;
    int sock = create_udp_socket("0.0.0.0", "0", &local_addr, &local_addr_len);
    if (sock < 0) return 0;

    quiche_conn *conn = quic_connect(config, sock, &local_addr, local_addr_len,
                                      peer_addr, peer_addr_len);
    if (!conn) { close(sock); return 0; }

    fprintf(stderr, "[producer] Connected to coordinator\n");

    // Open a bidirectional stream (client-initiated bidi stream IDs: 0, 4, 8, ...)
    Message msg = { .type = MSG_SUBMIT, .task_id = id, .duration = duration };
    strncpy(msg.description, desc, DESC_SIZE - 1);
    msg.description[DESC_SIZE - 1] = '\0';

    uint64_t stream_id = 0;  // First client-initiated bidi stream
    uint64_t err = 0;
    ssize_t sent = quiche_conn_stream_send(conn, stream_id,
                                            (const uint8_t *)&msg, sizeof(msg),
                                            true, &err);
    if (sent < 0) {
        fprintf(stderr, "[producer] stream_send failed: %zd\n", sent);
        quiche_conn_close(conn, true, 0, NULL, 0);
        flush_egress(sock, conn);
        quiche_conn_free(conn);
        close(sock);
        return 0;
    }

    flush_egress(sock, conn);

    // Wait for response on the same stream
    struct pollfd pfd = { .fd = sock, .events = POLLIN };
    uint8_t resp_buf[512];
    ssize_t resp_len = -1;
    bool got_response = false;

    for (int i = 0; i < 50 && !got_response; i++) {
        int timeout = (int)quiche_conn_timeout_as_millis(conn);
        if (timeout < 0 || timeout > 200) timeout = 200;
        poll(&pfd, 1, timeout);

        quiche_conn_on_timeout(conn);
        if (drive_conn(sock, conn, &local_addr, local_addr_len) < 0) break;

        bool fin = false;
        resp_len = quiche_conn_stream_recv(conn, stream_id,
                                            resp_buf, sizeof(resp_buf),
                                            &fin, &err);
        if (resp_len > 0) got_response = true;
    }

    if (got_response && resp_len >= 3 && memcmp(resp_buf, "ACK", 3) == 0) {
        printf("[producer] Task %d accepted: \"%s\" (%ds)\n", id, desc, duration);
    } else {
        printf("[producer] Task %d: no ACK received.\n", id);
    }

    // Close cleanly
    quiche_conn_close(conn, true, 0, NULL, 0);
    flush_egress(sock, conn);
    quiche_conn_free(conn);
    close(sock);
    return 1;
}

// =============================================================================
// MAIN
// =============================================================================
int main(int argc, char *argv[]) {
    srand(time(NULL));

    quiche_config *config = create_client_config();
    if (!config) { fprintf(stderr, "Failed to create config\n"); return 1; }

    // Resolve coordinator address
    struct addrinfo hints = {
        .ai_family   = PF_UNSPEC,
        .ai_socktype = SOCK_DGRAM,
        .ai_protocol = IPPROTO_UDP,
    };
    struct addrinfo *peer;
    if (getaddrinfo(SERVER_IP, QUICHE_PORT_STR, &hints, &peer) != 0) {
        perror("getaddrinfo");
        return 1;
    }

    printf("=== QUICHE PRODUCER (using quiche %s) ===\n\n", quiche_version());

    if (argc > 1) {
        int count = atoi(argv[1]);
        if (count <= 0) count = 5;
        printf("[producer] Auto-generating %d tasks...\n", count);
        for (int i = 1; i <= count; i++) {
            char desc[DESC_SIZE];
            snprintf(desc, DESC_SIZE, "QUICHE-Task-%d", i);
            submit_task(config, peer->ai_addr, peer->ai_addrlen,
                        i, 2 + rand() % 6, desc);
        }
    } else {
        printf("[producer] Interactive mode. Type description or 'quit':\n");
        int id = 1;
        char line[256];
        while (1) {
            printf("> "); fflush(stdout);
            if (!fgets(line, sizeof(line), stdin)) break;
            line[strcspn(line, "\n")] = '\0';
            if (strcmp(line, "quit") == 0 || strcmp(line, "q") == 0) break;
            const char *desc = strlen(line) > 0 ? line : "Default-Task";
            submit_task(config, peer->ai_addr, peer->ai_addrlen,
                        id++, 2 + rand() % 6, desc);
        }
    }

    freeaddrinfo(peer);
    quiche_config_free(config);
    printf("[producer] Done.\n");
    return 0;
}
