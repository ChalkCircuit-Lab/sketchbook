/*
 * server.c – Transfer benchmark server
 * PCD / MISS1202O2 – Concurrent and Distributed Programming
 *
 * Usage:
 *   ./server -p tcp|udp|quic [options]
 *
 *   -p  Protocol : tcp, udp, quic
 *   -P  Port     : listen port (default: 9999)
 *   -c  TLS cert : path to PEM certificate  (QUIC only, default: server.cert)
 *   -k  TLS key  : path to PEM private key  (QUIC only, default: server.key)
 *
 * The server loops indefinitely, accepting one session at a time.
 * Press Ctrl-C to stop.
 */

#include "common.h"
#include <getopt.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/quic.h>

/* ── statistics output ───────────────────────────────────────────────────── */
static void print_stats(const char *proto, const char *mode_str,
                         uint64_t msgs, uint64_t bytes,
                         uint64_t lost, const unsigned char hash[32])
{
    printf("\n=== Server Statistics (%s – %s) ===\n", proto, mode_str);
    printf("Total messages received : %lu\n", (unsigned long)msgs);
    printf("Total bytes received    : %lu\n", (unsigned long)bytes);
    if (lost > 0)
        printf("Packets lost/out-order  : %lu\n", (unsigned long)lost);
    print_hash("SHA-256 (received)     ", hash);
    printf("Ready for next session.\n\n");
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* TCP server                                                                  */
/* ═══════════════════════════════════════════════════════════════════════════ */
static int tcp_server(uint16_t port)
{
    int lsock = socket(AF_INET, SOCK_STREAM, 0);
    if (lsock < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(lsock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in sa = {0};
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons(port);

    if (bind(lsock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind"); close(lsock); return -1;
    }
    if (listen(lsock, 5) < 0) {
        perror("listen"); close(lsock); return -1;
    }
    printf("TCP server listening on port %u\n\n", port);

    for (;;) {
        struct sockaddr_in cli = {0};
        socklen_t clen = sizeof(cli);
        int csock = accept(lsock, (struct sockaddr *)&cli, &clen);
        if (csock < 0) { perror("accept"); continue; }
        printf("Client connected: %s\n", inet_ntoa(cli.sin_addr));

        /* Session header */
        SessionHdr sh;
        if (recv_all(csock, &sh, sizeof(sh)) < 0 ||
            memcmp(sh.magic, "CDP1", 4) != 0) {
            fprintf(stderr, "Bad or missing session header – dropping\n");
            close(csock); continue;
        }
        uint32_t bsz    = ntohl(sh.block_size);
        uint64_t total  = be64toh(sh.total_bytes);
        uint8_t  mode   = sh.mode;
        const char *mstr = (mode == MODE_STREAMING) ? "streaming" : "stop-and-wait";

        printf("Session: block=%u B  total=%lu MB  mode=%s\n",
               bsz, (unsigned long)(total / (1024*1024)), mstr);

        uint8_t *buf = malloc(bsz);
        if (!buf) { close(csock); continue; }
        EVP_MD_CTX *sha = sha256_init();

        uint64_t recv_bytes = 0, msgs = 0;

        while (recv_bytes < total) {
            BlockHdr bh;
            if (recv_all(csock, &bh, sizeof(bh)) < 0) break;

            uint32_t pay = ntohl(bh.payload_size);
            if (pay == 0 || pay > bsz) {
                fprintf(stderr, "Invalid payload size %u – aborting\n", pay); break;
            }
            if (recv_all(csock, buf, pay) < 0) break;

            sha256_update(sha, buf, pay);
            recv_bytes += pay; msgs++;

            if (mode == MODE_STOP_WAIT) {
                /* Echo the seq_num back in network byte order unchanged */
                Ack ack;
                ack.seq_num = bh.seq_num;
                ack.ok      = 1;
                send_all(csock, &ack, sizeof(ack));
            }
            if (bh.is_last) break;
        }

        unsigned char hash[32];
        sha256_final(sha, hash);
        print_stats("TCP", mstr, msgs, recv_bytes, 0, hash);
        free(buf);
        close(csock);
    }

    close(lsock);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* UDP server                                                                  */
/* ═══════════════════════════════════════════════════════════════════════════ */
static int udp_server(uint16_t port)
{
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return -1; }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    int buf8m = 8 * 1024 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buf8m, sizeof(buf8m));
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buf8m, sizeof(buf8m));

    struct sockaddr_in sa = {0};
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons(port);

    if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind"); close(sock); return -1;
    }
    printf("UDP server listening on port %u\n\n", port);

    /* Max datagram = UdpBlockHdr + max safe UDP data payload */
    uint8_t dgram[sizeof(UdpBlockHdr) + UDP_MAX_BLOCK];

    for (;;) {
        struct sockaddr_in cli = {0};
        socklen_t clen = sizeof(cli);

        /* Wait for a session-start datagram */
        ssize_t n;
        for (;;) {
            n = recvfrom(sock, dgram, sizeof(dgram), 0,
                         (struct sockaddr *)&cli, &clen);
            if (n >= (ssize_t)sizeof(UdpSessionPkt) &&
                dgram[0] == UDP_SESSION)
                break;
        }

        UdpSessionPkt *sp = (UdpSessionPkt *)dgram;
        uint32_t bsz    = ntohl(sp->block_size);
        uint64_t total  = be64toh(sp->total_bytes);
        uint8_t  mode   = sp->mode;
        const char *mstr = (mode == MODE_STREAMING) ? "streaming" : "stop-and-wait";

        printf("Client: %s | block=%u B  total=%lu MB  mode=%s\n",
               inet_ntoa(cli.sin_addr), bsz,
               (unsigned long)(total / (1024*1024)), mstr);

        /* Session ACK */
        UdpAckPkt sess_ack;
        sess_ack.type    = UDP_ACK;
        sess_ack.seq_num = htobe64(UINT64_MAX);
        sess_ack.ok      = 1;
        sendto(sock, &sess_ack, sizeof(sess_ack), 0,
               (struct sockaddr *)&cli, clen);

        /* Set receive timeout – detect streaming session end */
        struct timeval tv = {5, 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

        uint8_t *buf = malloc(bsz + sizeof(UdpBlockHdr));
        if (!buf) continue;
        EVP_MD_CTX *sha = sha256_init();

        uint64_t expected_seq = 0, recv_bytes = 0, msgs = 0, lost = 0;

        for (;;) {
            n = recvfrom(sock, buf, bsz + sizeof(UdpBlockHdr), 0,
                         (struct sockaddr *)&cli, &clen);
            if (n <= 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    printf("UDP: receive timeout – assuming session complete\n");
                } else {
                    perror("recvfrom");
                }
                break;
            }
            if ((size_t)n < sizeof(UdpBlockHdr) || buf[0] != UDP_BLOCK)
                continue;

            UdpBlockHdr *bh  = (UdpBlockHdr *)buf;
            uint64_t seq      = be64toh(bh->seq_num);
            uint32_t pay      = ntohl(bh->payload_size);
            uint8_t *data     = buf + sizeof(UdpBlockHdr);

            /* Detect gaps (packet loss or reordering) */
            if (seq > expected_seq)
                lost += seq - expected_seq;
            expected_seq = seq + 1;

            sha256_update(sha, data, pay);
            recv_bytes += pay; msgs++;

            if (mode == MODE_STOP_WAIT) {
                UdpAckPkt ack;
                ack.type    = UDP_ACK;
                ack.seq_num = bh->seq_num; /* already in network byte order */
                ack.ok      = 1;
                sendto(sock, &ack, sizeof(ack), 0,
                       (struct sockaddr *)&cli, clen);
            }
            if (bh->is_last) break;
        }

        /* Clear receive timeout for next session-start wait */
        struct timeval notv = {0, 0};
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &notv, sizeof(notv));

        unsigned char hash[32];
        sha256_final(sha, hash);
        print_stats("UDP", mstr, msgs, recv_bytes, lost, hash);
        free(buf);
    }

    close(sock);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* QUIC helpers                                                                */
/* ═══════════════════════════════════════════════════════════════════════════ */
static int quic_read_all(SSL *ssl, void *buf, size_t n)
{
    char *p = (char *)buf;
    while (n > 0) {
        size_t r = 0;
        if (!SSL_read_ex(ssl, p, n, &r) || r == 0) return -1;
        p += r; n -= r;
    }
    return 0;
}

static int quic_write_all(SSL *ssl, const void *buf, size_t n)
{
    const char *p = (const char *)buf;
    while (n > 0) {
        size_t w = 0;
        if (!SSL_write_ex(ssl, p, n, &w)) return -1;
        p += w; n -= w;
    }
    return 0;
}

/* ALPN select callback for the QUIC server: accepts "cdp1". */
static int select_alpn(SSL *ssl, const unsigned char **out, unsigned char *outlen,
                         const unsigned char *in, unsigned int inlen, void *arg)
{
    (void)ssl; (void)arg;
    /*
     * The list is in NPN/ALPN wire format: each entry is a 1-byte length
     * followed by the protocol name bytes.  Pass the full array including
     * the length prefix.
     */
    static const unsigned char supported[] = { 4, 'c', 'd', 'p', '1' };
    if (SSL_select_next_proto((unsigned char **)out, outlen,
                               supported, sizeof(supported),
                               in, inlen) == OPENSSL_NPN_NEGOTIATED)
        return SSL_TLSEXT_ERR_OK;
    return SSL_TLSEXT_ERR_ALERT_FATAL;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* QUIC server                                                                 */
/* ═══════════════════════════════════════════════════════════════════════════ */
static int quic_server(uint16_t port,
                        const char *cert_path, const char *key_path)
{
    SSL_CTX *ctx = SSL_CTX_new(OSSL_QUIC_server_method());
    if (!ctx) { ERR_print_errors_fp(stderr); return -1; }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    if (SSL_CTX_use_certificate_chain_file(ctx, cert_path) <= 0) {
        fprintf(stderr, "Cannot load certificate: %s\n", cert_path);
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx); return -1;
    }
    if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
        fprintf(stderr, "Cannot load private key: %s\n", key_path);
        ERR_print_errors_fp(stderr);
        SSL_CTX_free(ctx); return -1;
    }
    SSL_CTX_set_alpn_select_cb(ctx, select_alpn, NULL);

    /* UDP socket for QUIC */
    int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0) { perror("socket"); SSL_CTX_free(ctx); return -1; }

    struct sockaddr_in sa = {0};
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = INADDR_ANY;
    sa.sin_port        = htons(port);
    if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("bind"); close(fd); SSL_CTX_free(ctx); return -1;
    }

    SSL *listener = SSL_new_listener(ctx, 0);
    if (!listener) {
        fprintf(stderr, "SSL_new_listener failed\n");
        ERR_print_errors_fp(stderr);
        close(fd); SSL_CTX_free(ctx); return -1;
    }
    if (!SSL_set_fd(listener, fd) || !SSL_listen(listener)) {
        fprintf(stderr, "QUIC listen failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(listener); close(fd); SSL_CTX_free(ctx); return -1;
    }
    printf("QUIC server listening on UDP port %u\n\n", port);

    for (;;) {
        ERR_clear_error();
        SSL *conn = SSL_accept_connection(listener, 0);
        if (!conn) {
            fprintf(stderr, "SSL_accept_connection failed\n");
            ERR_print_errors_fp(stderr);
            continue;
        }
        printf("QUIC client connected\n");

        /*
         * The client writes to its default stream (a client-initiated
         * bidirectional stream). On the server side, SSL_accept_connection()
         * returns a connection that has no default stream; we must call
         * SSL_accept_stream() to obtain the client's incoming stream.
         */
        SSL *stream = SSL_accept_stream(conn, 0);
        if (!stream) {
            fprintf(stderr, "QUIC: SSL_accept_stream failed\n");
            ERR_print_errors_fp(stderr);
            SSL_free(conn); continue;
        }

        /* Session header */
        SessionHdr sh;
        if (quic_read_all(stream, &sh, sizeof(sh)) < 0 ||
            memcmp(sh.magic, "CDP1", 4) != 0) {
            fprintf(stderr, "QUIC: bad session header – dropping\n");
            SSL_free(stream); SSL_free(conn); continue;
        }
        uint32_t bsz    = ntohl(sh.block_size);
        uint64_t total  = be64toh(sh.total_bytes);
        uint8_t  mode   = sh.mode;
        const char *mstr = (mode == MODE_STREAMING) ? "streaming" : "stop-and-wait";

        printf("Session: block=%u B  total=%lu MB  mode=%s\n",
               bsz, (unsigned long)(total / (1024*1024)), mstr);

        uint8_t *buf = malloc(bsz);
        if (!buf) { SSL_free(stream); SSL_free(conn); continue; }
        EVP_MD_CTX *sha = sha256_init();

        uint64_t recv_bytes = 0, msgs = 0;

        while (recv_bytes < total) {
            BlockHdr bh;
            if (quic_read_all(stream, &bh, sizeof(bh)) < 0) break;

            uint32_t pay = ntohl(bh.payload_size);
            if (pay == 0 || pay > bsz) break;
            if (quic_read_all(stream, buf, pay) < 0) break;

            sha256_update(sha, buf, pay);
            recv_bytes += pay; msgs++;

            if (mode == MODE_STOP_WAIT) {
                Ack ack;
                ack.seq_num = bh.seq_num; /* already in network byte order */
                ack.ok      = 1;
                quic_write_all(stream, &ack, sizeof(ack));
            }
            if (bh.is_last) break;
        }

        unsigned char hash[32];
        sha256_final(sha, hash);
        print_stats("QUIC", mstr, msgs, recv_bytes, 0, hash);
        free(buf);

        SSL_stream_conclude(stream, 0);
        SSL_free(stream);
        while (SSL_shutdown(conn) == 0)
            ;
        SSL_free(conn);
    }

    SSL_free(listener);
    close(fd);
    SSL_CTX_free(ctx);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* main                                                                        */
/* ═══════════════════════════════════════════════════════════════════════════ */
static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s -p tcp|udp|quic [options]\n"
        "  -p  Protocol : tcp, udp, quic\n"
        "  -P  Port     (default: %d)\n"
        "  -c  TLS cert (default: server.cert)  [QUIC only]\n"
        "  -k  TLS key  (default: server.key)   [QUIC only]\n",
        prog, DEFAULT_PORT);
}

int main(int argc, char *argv[])
{
    const char *proto     = NULL;
    const char *cert_path = "server.cert";
    const char *key_path  = "server.key";
    uint16_t    port      = DEFAULT_PORT;
    int c;

    while ((c = getopt(argc, argv, "p:P:c:k:h")) != -1) {
        switch (c) {
        case 'p': proto     = optarg; break;
        case 'P': port      = (uint16_t)atoi(optarg); break;
        case 'c': cert_path = optarg; break;
        case 'k': key_path  = optarg; break;
        default:  usage(argv[0]); return 1;
        }
    }

    if (!proto) {
        fprintf(stderr, "Error: -p is required.\n");
        usage(argv[0]); return 1;
    }

    signal(SIGPIPE, SIG_IGN); /* ignore broken-pipe on disconnected clients */

    if      (strcmp(proto, "tcp")  == 0) return tcp_server (port);
    else if (strcmp(proto, "udp")  == 0) return udp_server (port);
    else if (strcmp(proto, "quic") == 0) return quic_server(port, cert_path, key_path);
    else { fprintf(stderr, "Unknown protocol: %s\n", proto); return 1; }
}
