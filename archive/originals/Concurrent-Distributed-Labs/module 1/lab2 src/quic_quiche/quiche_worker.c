// =============================================================================
// QUIC WORKER (Consumer) -- using Cloudflare quiche
//
// Requests and processes tasks from the QUIC Coordinator.
// Equivalent to ../quic/quic_worker.c (MsQuic version).
//
// Flow per iteration:
//   1. Open QUIC connection to coordinator
//   2. Open bidi stream, send MSG_READY, read TaskResponse
//   3. If has_task: close connection, sleep(duration), reconnect, send MSG_DONE
//   4. If no task: close connection, wait 3s, retry
//
// Usage: ./quiche_worker
// =============================================================================

#include "quiche_common.h"

// --- Drive a connection: recv from socket -> feed to quiche -> flush egress ---
static int drive_conn(int sock, quiche_conn *conn,
                      struct sockaddr_storage *local_addr,
                      socklen_t local_addr_len) {
    static uint8_t buf[65535];

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
            fprintf(stderr, "[worker] quiche_conn_recv: %zd\n", done);
            return -1;
        }
    }

    flush_egress(sock, conn);

    if (quiche_conn_is_closed(conn)) return -1;
    return 0;
}

// --- Connect to coordinator, complete handshake ---
static quiche_conn *quic_connect(quiche_config *config, int sock,
                                  struct sockaddr_storage *local_addr,
                                  socklen_t local_addr_len,
                                  struct sockaddr *peer_addr,
                                  socklen_t peer_addr_len) {
    uint8_t scid[LOCAL_CONN_ID_LEN];
    if (gen_cid(scid, sizeof(scid)) < 0) return NULL;

    quiche_conn *conn = quiche_connect(SERVER_IP,
                                        scid, sizeof(scid),
                                        (struct sockaddr *)local_addr,
                                        local_addr_len,
                                        peer_addr, peer_addr_len,
                                        config);
    if (!conn) {
        fprintf(stderr, "[worker] quiche_connect failed\n");
        return NULL;
    }

    flush_egress(sock, conn);

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

// --- Send a Message and receive a response over a single QUIC exchange ---
// Returns response length, or -1 on failure.  Response is in resp_buf.
static ssize_t quic_request(quiche_config *config,
                             struct sockaddr *peer_addr,
                             socklen_t peer_addr_len,
                             const Message *msg,
                             uint8_t *resp_buf, size_t resp_buf_len) {
    // Fresh socket per request
    struct sockaddr_storage local_addr;
    socklen_t local_addr_len;
    int sock = create_udp_socket("0.0.0.0", "0", &local_addr, &local_addr_len);
    if (sock < 0) return -1;

    quiche_conn *conn = quic_connect(config, sock,
                                      &local_addr, local_addr_len,
                                      peer_addr, peer_addr_len);
    if (!conn) { close(sock); return -1; }

    // Send on stream 0 (first client-initiated bidi)
    uint64_t stream_id = 0;
    uint64_t err = 0;
    ssize_t sent = quiche_conn_stream_send(conn, stream_id,
                                            (const uint8_t *)msg, sizeof(*msg),
                                            true, &err);
    if (sent < 0) {
        fprintf(stderr, "[worker] stream_send failed: %zd\n", sent);
        quiche_conn_close(conn, true, 0, NULL, 0);
        flush_egress(sock, conn);
        quiche_conn_free(conn);
        close(sock);
        return -1;
    }

    flush_egress(sock, conn);

    // Wait for response
    struct pollfd pfd = { .fd = sock, .events = POLLIN };
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
                                            resp_buf, resp_buf_len,
                                            &fin, &err);
        if (resp_len > 0) got_response = true;
    }

    // Close cleanly
    quiche_conn_close(conn, true, 0, NULL, 0);
    flush_egress(sock, conn);
    quiche_conn_free(conn);
    close(sock);

    return got_response ? resp_len : -1;
}

// =============================================================================
// MAIN
// =============================================================================
int main(void) {
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

    printf("=== QUICHE WORKER started (using quiche %s) ===\n\n",
           quiche_version());

    while (1) {
        // 1. Request work
        Message req = { .type = MSG_READY };
        uint8_t resp_buf[512];
        ssize_t rlen = quic_request(config, peer->ai_addr, peer->ai_addrlen,
                                     &req, resp_buf, sizeof(resp_buf));

        if (rlen < (ssize_t)sizeof(TaskResponse)) {
            printf("[worker] Cannot reach coordinator. Retrying in 2s...\n");
            sleep(2);
            continue;
        }

        TaskResponse resp;
        memcpy(&resp, resp_buf, sizeof(resp));

        if (!resp.has_task) {
            printf("[worker] No tasks available. Idle for 3s...\n");
            sleep(3);
            continue;
        }

        // 2. Process the task
        resp.description[DESC_SIZE - 1] = '\0';
        printf("[worker] Processing task %d: \"%s\" (%ds)...\n",
               resp.task_id, resp.description, resp.duration);
        sleep(resp.duration);
        printf("[worker] Task %d DONE.\n", resp.task_id);

        // 3. Report completion
        Message done = { .type = MSG_DONE, .task_id = resp.task_id };
        ssize_t ack_len = quic_request(config, peer->ai_addr, peer->ai_addrlen,
                                        &done, resp_buf, sizeof(resp_buf));
        if (ack_len > 0) {
            printf("[worker] Completion reported for task %d.\n", resp.task_id);
        }
    }

    freeaddrinfo(peer);
    quiche_config_free(config);
    return 0;
}
