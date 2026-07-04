// =============================================================================
// QUIC COORDINATOR (Broker) -- using Cloudflare quiche
//
// Single-threaded event-loop server.  Unlike the MsQuic version (callback-
// driven), here every QUIC operation is explicit:
//
//   1. Receive UDP datagrams with recvfrom()
//   2. Parse the QUIC header to find the destination connection ID
//   3. Look up (or create) the corresponding quiche_conn
//   4. Feed the datagram to quiche_conn_recv()
//   5. Iterate readable streams, process application messages
//   6. Call quiche_conn_send() to produce outgoing packets, sendto() them
//   7. Handle timeouts, clean up closed connections
//
// This is the most complex component because it handles:
//   - Version negotiation
//   - Stateless retry tokens (address validation)
//   - Connection lifecycle management
//   - Stream multiplexing
//   - Task queue logic
//
// Usage: ./quiche_coordinator [cert.pem] [key.pem]
// =============================================================================

#include "quiche_common.h"

// --- Tunables ---
#define MAX_CONNECTIONS 64
#define QUEUE_SIZE      100

// --- Token helpers (stateless retry) ---
// Tokens embed: "quiche" prefix + client address + original DCID.
// This is the same scheme used by the official quiche server example.

#define MAX_TOKEN_LEN \
    (sizeof("quiche") - 1 + sizeof(struct sockaddr_storage) + QUICHE_MAX_CONN_ID_LEN)

static void mint_token(const uint8_t *dcid, size_t dcid_len,
                       struct sockaddr_storage *addr, socklen_t addr_len,
                       uint8_t *token, size_t *token_len) {
    memcpy(token, "quiche", sizeof("quiche") - 1);
    memcpy(token + sizeof("quiche") - 1, addr, addr_len);
    memcpy(token + sizeof("quiche") - 1 + addr_len, dcid, dcid_len);
    *token_len = sizeof("quiche") - 1 + addr_len + dcid_len;
}

static bool validate_token(const uint8_t *token, size_t token_len,
                           struct sockaddr_storage *addr, socklen_t addr_len,
                           uint8_t *odcid, size_t *odcid_len) {
    if (token_len < sizeof("quiche") - 1 ||
        memcmp(token, "quiche", sizeof("quiche") - 1) != 0)
        return false;

    token     += sizeof("quiche") - 1;
    token_len -= sizeof("quiche") - 1;

    if (token_len < (size_t)addr_len ||
        memcmp(token, addr, addr_len) != 0)
        return false;

    token     += addr_len;
    token_len -= addr_len;

    if (*odcid_len < token_len)
        return false;

    memcpy(odcid, token, token_len);
    *odcid_len = token_len;
    return true;
}

// --- Connection Table ---
// Simple fixed-size array.  For a lab exercise this is sufficient;
// a production server would use a hash table (e.g. uthash).

typedef struct {
    quiche_conn *conn;
    struct sockaddr_storage peer_addr;
    socklen_t peer_addr_len;
    uint8_t cid[LOCAL_CONN_ID_LEN];
} ConnEntry;

static ConnEntry conn_table[MAX_CONNECTIONS];
static int       conn_count = 0;

static ConnEntry *find_conn(const uint8_t *dcid, size_t dcid_len) {
    for (int i = 0; i < conn_count; i++) {
        if (dcid_len == LOCAL_CONN_ID_LEN &&
            memcmp(conn_table[i].cid, dcid, LOCAL_CONN_ID_LEN) == 0)
            return &conn_table[i];
    }
    return NULL;
}

static ConnEntry *add_conn(quiche_conn *conn,
                           const uint8_t *cid,
                           struct sockaddr_storage *peer,
                           socklen_t peer_len) {
    if (conn_count >= MAX_CONNECTIONS) {
        fprintf(stderr, "[coord] Connection table full!\n");
        return NULL;
    }
    ConnEntry *e = &conn_table[conn_count++];
    e->conn = conn;
    memcpy(e->cid, cid, LOCAL_CONN_ID_LEN);
    memcpy(&e->peer_addr, peer, peer_len);
    e->peer_addr_len = peer_len;
    return e;
}

static void remove_conn(int idx) {
    quiche_conn_free(conn_table[idx].conn);
    conn_table[idx] = conn_table[conn_count - 1];
    conn_count--;
}

// --- Task Queue (same as TCP/MsQuic versions) ---
typedef struct {
    Message tasks[QUEUE_SIZE];
    int head, tail, count;
    pthread_mutex_t lock;
} TaskQueue;

static TaskQueue queue;
static int next_task_id = 1;

static void queue_init(void) {
    queue.head = queue.tail = queue.count = 0;
    pthread_mutex_init(&queue.lock, NULL);
}

static void enqueue(Message t) {
    pthread_mutex_lock(&queue.lock);
    if (queue.count < QUEUE_SIZE) {
        queue.tasks[queue.tail] = t;
        queue.tail = (queue.tail + 1) % QUEUE_SIZE;
        queue.count++;
        printf("[coord] Task %d enqueued (\"%s\"). Queue: %d\n",
               t.task_id, t.description, queue.count);
    } else {
        printf("[coord] Queue FULL! Task %d dropped.\n", t.task_id);
    }
    pthread_mutex_unlock(&queue.lock);
}

static int dequeue(Message *t) {
    pthread_mutex_lock(&queue.lock);
    int ok = 0;
    if (queue.count > 0) {
        *t = queue.tasks[queue.head];
        queue.head = (queue.head + 1) % QUEUE_SIZE;
        queue.count--;
        ok = 1;
    }
    pthread_mutex_unlock(&queue.lock);
    return ok;
}

// --- Process readable streams on an established connection ---
static void process_streams(quiche_conn *conn) {
    uint64_t stream_id;
    quiche_stream_iter *readable = quiche_conn_readable(conn);

    while (quiche_stream_iter_next(readable, &stream_id)) {
        static uint8_t buf[65535];
        bool fin = false;
        uint64_t error_code = 0;

        ssize_t recv_len = quiche_conn_stream_recv(conn, stream_id,
                                                    buf, sizeof(buf),
                                                    &fin, &error_code);
        if (recv_len < 0) continue;

        if ((size_t)recv_len < sizeof(Message)) {
            fprintf(stderr, "[coord] Short message on stream %" PRIu64 " (%zd bytes)\n",
                    stream_id, recv_len);
            continue;
        }

        Message msg;
        memcpy(&msg, buf, sizeof(msg));
        msg.description[DESC_SIZE - 1] = '\0';

        uint64_t err = 0;

        switch (msg.type) {

        case MSG_SUBMIT: {
            msg.task_id = next_task_id++;
            enqueue(msg);
            printf("[coord] Producer submitted task %d: \"%s\" (%ds)\n",
                   msg.task_id, msg.description, msg.duration);

            // ACK back on the same stream (with FIN)
            const char *ack = "ACK";
            quiche_conn_stream_send(conn, stream_id,
                                    (const uint8_t *)ack, 3, true, &err);
            break;
        }

        case MSG_READY: {
            TaskResponse resp = {0};
            Message t;
            if (dequeue(&t)) {
                resp.has_task  = 1;
                resp.task_id   = t.task_id;
                resp.duration  = t.duration;
                strncpy(resp.description, t.description, DESC_SIZE);
                printf("[coord] Assigned task %d to worker.\n", t.task_id);
            } else {
                resp.has_task = 0;
                printf("[coord] No tasks. Worker goes idle.\n");
            }
            quiche_conn_stream_send(conn, stream_id,
                                    (const uint8_t *)&resp, sizeof(resp),
                                    true, &err);
            break;
        }

        case MSG_DONE: {
            printf("[coord] Task %d completed by worker.\n", msg.task_id);
            const char *ok = "OK";
            quiche_conn_stream_send(conn, stream_id,
                                    (const uint8_t *)ok, 2, true, &err);
            break;
        }

        default:
            printf("[coord] Unknown message type %d on stream %" PRIu64 "\n",
                   msg.type, stream_id);
        }
    }

    quiche_stream_iter_free(readable);
}

// =============================================================================
// MAIN
// =============================================================================
int main(int argc, char *argv[]) {
    const char *cert = (argc > 1) ? argv[1] : "server.cert";
    const char *key  = (argc > 2) ? argv[2] : "server.key";

    queue_init();

    // --- Create quiche config ---
    quiche_config *config = create_server_config(cert, key);
    if (!config) {
        fprintf(stderr, "Failed to create quiche config\n");
        return 1;
    }

    // --- Create and bind UDP socket ---
    struct sockaddr_storage local_addr;
    socklen_t local_addr_len;
    int sock = create_udp_socket(SERVER_IP, QUICHE_PORT_STR,
                                 &local_addr, &local_addr_len);
    if (sock < 0) return 1;

    printf("=== QUICHE COORDINATOR listening on %s:%d ===\n",
           SERVER_IP, QUICHE_PORT);
    printf("    (TLS 1.3 encrypted, cert: %s)\n", cert);
    printf("    Using quiche %s\n\n", quiche_version());

    // --- Event loop ---
    static uint8_t buf[65535];
    static uint8_t out[MAX_DATAGRAM_SIZE];

    for (;;) {
        // Compute the minimum timeout across all connections
        int timeout_ms = 100; // default poll timeout
        for (int i = 0; i < conn_count; i++) {
            uint64_t t = quiche_conn_timeout_as_millis(conn_table[i].conn);
            if ((int)t < timeout_ms) timeout_ms = (int)t;
        }

        struct pollfd pfd = { .fd = sock, .events = POLLIN };
        poll(&pfd, 1, timeout_ms);

        // --- Receive incoming UDP datagrams ---
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
                break;
            }

            // Parse QUIC header
            uint8_t type;
            uint32_t version;
            uint8_t scid[QUICHE_MAX_CONN_ID_LEN], dcid[QUICHE_MAX_CONN_ID_LEN];
            size_t  scid_len = sizeof(scid),        dcid_len = sizeof(dcid);
            uint8_t token[MAX_TOKEN_LEN];
            size_t  token_len = sizeof(token);

            int rc = quiche_header_info(buf, read_len, LOCAL_CONN_ID_LEN,
                                        &version, &type,
                                        scid, &scid_len,
                                        dcid, &dcid_len,
                                        token, &token_len);
            if (rc < 0) {
                fprintf(stderr, "[coord] Failed to parse header: %d\n", rc);
                continue;
            }

            // Look up existing connection by destination CID
            ConnEntry *entry = find_conn(dcid, dcid_len);

            if (entry == NULL) {
                // --- New connection ---

                // Version negotiation
                if (!quiche_version_is_supported(version)) {
                    fprintf(stderr, "[coord] Version negotiation\n");
                    ssize_t written = quiche_negotiate_version(
                        scid, scid_len, dcid, dcid_len, out, sizeof(out));
                    if (written < 0) {
                        fprintf(stderr, "[coord] negotiate_version failed: %zd\n", written);
                        continue;
                    }
                    sendto(sock, out, written, 0,
                           (struct sockaddr *)&peer_addr, peer_addr_len);
                    continue;
                }

                // Stateless retry: if no token, send a Retry packet
                if (token_len == 0) {
                    fprintf(stderr, "[coord] Stateless retry\n");

                    uint8_t new_cid[LOCAL_CONN_ID_LEN];
                    if (gen_cid(new_cid, sizeof(new_cid)) < 0) continue;

                    uint8_t retry_token[MAX_TOKEN_LEN];
                    size_t  retry_token_len;
                    mint_token(dcid, dcid_len, &peer_addr, peer_addr_len,
                               retry_token, &retry_token_len);

                    ssize_t written = quiche_retry(
                        scid, scid_len,
                        dcid, dcid_len,
                        new_cid, LOCAL_CONN_ID_LEN,
                        retry_token, retry_token_len,
                        version, out, sizeof(out));

                    if (written < 0) {
                        fprintf(stderr, "[coord] quiche_retry failed: %zd\n", written);
                        continue;
                    }
                    sendto(sock, out, written, 0,
                           (struct sockaddr *)&peer_addr, peer_addr_len);
                    continue;
                }

                // Validate the retry token
                uint8_t odcid[QUICHE_MAX_CONN_ID_LEN];
                size_t  odcid_len = sizeof(odcid);

                if (!validate_token(token, token_len,
                                    &peer_addr, peer_addr_len,
                                    odcid, &odcid_len)) {
                    fprintf(stderr, "[coord] Invalid token\n");
                    continue;
                }

                // Accept the connection
                quiche_conn *conn = quiche_accept(
                    dcid, dcid_len,
                    odcid, odcid_len,
                    (struct sockaddr *)&local_addr, local_addr_len,
                    (struct sockaddr *)&peer_addr, peer_addr_len,
                    config);

                if (conn == NULL) {
                    fprintf(stderr, "[coord] quiche_accept failed\n");
                    continue;
                }

                entry = add_conn(conn, dcid, &peer_addr, peer_addr_len);
                if (!entry) {
                    quiche_conn_free(conn);
                    continue;
                }

                fprintf(stderr, "[coord] New connection accepted (total: %d)\n",
                        conn_count);
            }

            // --- Feed the datagram to the connection ---
            quiche_recv_info recv_info = {
                .from     = (struct sockaddr *)&peer_addr,
                .from_len = peer_addr_len,
                .to       = (struct sockaddr *)&local_addr,
                .to_len   = local_addr_len,
            };

            ssize_t done = quiche_conn_recv(entry->conn, buf, read_len,
                                            &recv_info);
            if (done < 0) {
                fprintf(stderr, "[coord] quiche_conn_recv failed: %zd\n", done);
                continue;
            }

            // --- Process readable streams ---
            if (quiche_conn_is_established(entry->conn)) {
                process_streams(entry->conn);
            }
        } // end recvfrom loop

        // --- Flush egress for all connections ---
        for (int i = 0; i < conn_count; i++) {
            flush_egress(sock, conn_table[i].conn);
        }

        // --- Handle timeouts and clean up closed connections ---
        for (int i = conn_count - 1; i >= 0; i--) {
            quiche_conn *c = conn_table[i].conn;

            // Process timeout
            quiche_conn_on_timeout(c);

            if (quiche_conn_is_closed(c)) {
                quiche_stats stats;
                quiche_conn_stats(c, &stats);

                fprintf(stderr, "[coord] Connection closed. recv=%zu sent=%zu lost=%zu\n",
                        stats.recv, stats.sent, stats.lost);

                remove_conn(i);
            }
        }
    }

    quiche_config_free(config);
    close(sock);
    return 0;
}
