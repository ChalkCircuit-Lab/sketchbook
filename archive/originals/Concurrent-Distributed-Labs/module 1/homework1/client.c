/*
 * client.c – Transfer benchmark client
 * PCD / MISS1202O2 – Concurrent and Distributed Programming
 *
 * Usage:
 *   ./client -p tcp|udp|quic -b BLOCK_SIZE -d DATA_MB [options]
 *
 *   -p  Protocol  : tcp, udp, quic
 *   -b  Block size: 1..65535 bytes
 *   -d  Data size : in MB  (e.g. 500 or 1024)
 *   -H  Host      : server IP/hostname  (default: 127.0.0.1)
 *   -P  Port      : server port         (default: 9999)
 *   -m  Mode      : streaming (default) or stop-wait
 *
 * Examples:
 *   ./client -p tcp  -b 1000  -d 500
 *   ./client -p udp  -b 65535 -d 1024 -m stop-wait
 *   ./client -p quic -b 10000 -d 500
 */

#include "common.h"
#include <getopt.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#include <openssl/quic.h>

/* ── statistics output ───────────────────────────────────────────────────── */
static void print_stats(const char *proto, const char *mode_str,
                         double elapsed, uint64_t msgs, uint64_t bytes,
                         const unsigned char hash[32])
{
    printf("\n=== Client Statistics (%s – %s) ===\n", proto, mode_str);
    printf("Total transmission time : %.3f s\n", elapsed);
    if (elapsed > 0)
        printf("Throughput              : %.2f MB/s\n",
               (bytes / 1048576.0) / elapsed);
    printf("Messages sent           : %lu\n", (unsigned long)msgs);
    printf("Bytes sent              : %lu\n", (unsigned long)bytes);
    print_hash("SHA-256 (sent data)    ", hash);
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* TCP client                                                                  */
/* ═══════════════════════════════════════════════════════════════════════════ */
static int tcp_client(const char *host, uint16_t port,
                       uint32_t bsz, uint64_t total, uint8_t mode)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("socket"); return -1; }

    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    if (inet_pton(AF_INET, host, &sa.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", host);
        close(sock); return -1;
    }
    if (connect(sock, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        perror("connect"); close(sock); return -1;
    }

    /* Session header */
    SessionHdr sh;
    memcpy(sh.magic, "CDP1", 4);
    sh.mode       = mode;
    sh.block_size  = htonl(bsz);
    sh.total_bytes = htobe64(total);
    if (send_all(sock, &sh, sizeof(sh)) < 0) {
        perror("send header"); close(sock); return -1;
    }

    uint8_t *buf = malloc(bsz);
    if (!buf) { close(sock); return -1; }
    EVP_MD_CTX *sha = sha256_init();

    double   t0 = now_sec();
    uint64_t seq = 0, sent_bytes = 0, msgs = 0;

    while (sent_bytes < total) {
        uint32_t pay = (uint32_t)((total - sent_bytes < bsz)
                                   ? (total - sent_bytes) : bsz);
        fill_data(buf, pay, sent_bytes);
        sha256_update(sha, buf, pay);

        BlockHdr bh;
        bh.seq_num      = htobe64(seq);
        bh.payload_size = htonl(pay);
        bh.is_last      = (sent_bytes + pay >= total) ? 1 : 0;

        if (send_all(sock, &bh, sizeof(bh)) < 0 ||
            send_all(sock, buf, pay) < 0) {
            perror("send block"); break;
        }

        if (mode == MODE_STOP_WAIT) {
            Ack ack;
            if (recv_all(sock, &ack, sizeof(ack)) < 0) {
                fprintf(stderr, "No ACK for seq %lu\n", (unsigned long)seq);
                break;
            }
        }
        sent_bytes += pay; msgs++; seq++;
    }

    double elapsed = now_sec() - t0;
    unsigned char hash[32];
    sha256_final(sha, hash);
    print_stats("TCP", mode == MODE_STREAMING ? "streaming" : "stop-and-wait",
                elapsed, msgs, sent_bytes, hash);

    free(buf);
    close(sock);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* UDP client                                                                  */
/* ═══════════════════════════════════════════════════════════════════════════ */
static int udp_client(const char *host, uint16_t port,
                       uint32_t bsz, uint64_t total, uint8_t mode)
{
    if (bsz > UDP_MAX_BLOCK) {
        fprintf(stderr,
            "UDP: block size %u exceeds the maximum UDP payload (%u bytes).\n"
            "     Use -b %u or smaller.\n",
            bsz, UDP_MAX_BLOCK, UDP_MAX_BLOCK);
        return -1;
    }

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) { perror("socket"); return -1; }

    /* Enlarge OS socket buffers for high-speed streaming */
    int buf4m = 4 * 1024 * 1024;
    setsockopt(sock, SOL_SOCKET, SO_SNDBUF, &buf4m, sizeof(buf4m));
    setsockopt(sock, SOL_SOCKET, SO_RCVBUF, &buf4m, sizeof(buf4m));

    /* Timeout for waiting on ACKs (session or block) */
    struct timeval tv = {2, 0};
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port   = htons(port);
    if (inet_pton(AF_INET, host, &sa.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address: %s\n", host);
        close(sock); return -1;
    }
    /* "connect" so we can use send/recv instead of sendto/recvfrom */
    connect(sock, (struct sockaddr *)&sa, sizeof(sa));

    /* ── Session handshake ─────────────────────────────────────────────── */
    UdpSessionPkt sp;
    sp.type       = UDP_SESSION;
    sp.mode       = mode;
    sp.block_size  = htonl(bsz);
    sp.total_bytes = htobe64(total);

    int ok = 0;
    for (int retry = 0; retry < 5 && !ok; retry++) {
        if (send(sock, &sp, sizeof(sp), 0) < 0) {
            perror("send session"); close(sock); return -1;
        }
        UdpAckPkt ack;
        if (recv(sock, &ack, sizeof(ack), 0) == (ssize_t)sizeof(ack) &&
            ack.type == UDP_ACK &&
            be64toh(ack.seq_num) == UINT64_MAX && ack.ok)
            ok = 1;
    }
    if (!ok) {
        fprintf(stderr, "UDP: no session ACK from server\n");
        close(sock); return -1;
    }

    /* ── Allocate datagram buffer (header + max payload) ──────────────── */
    size_t  dgram_sz = sizeof(UdpBlockHdr) + bsz;
    uint8_t *dgram   = malloc(dgram_sz);
    if (!dgram) { close(sock); return -1; }
    uint8_t *payload = dgram + sizeof(UdpBlockHdr);

    EVP_MD_CTX *sha = sha256_init();
    double   t0 = now_sec();
    uint64_t seq = 0, sent_bytes = 0, msgs = 0;

    while (sent_bytes < total) {
        uint32_t pay = (uint32_t)((total - sent_bytes < bsz)
                                   ? (total - sent_bytes) : bsz);
        fill_data(payload, pay, sent_bytes);
        sha256_update(sha, payload, pay);

        UdpBlockHdr *bh = (UdpBlockHdr *)dgram;
        bh->type         = UDP_BLOCK;
        bh->seq_num      = htobe64(seq);
        bh->payload_size = htonl(pay);
        bh->is_last      = (sent_bytes + pay >= total) ? 1 : 0;

        int acked   = 0;
        int retries = (mode == MODE_STOP_WAIT) ? 3 : 1;
        for (int r = 0; r < retries; r++) {
            ssize_t sent = send(sock, dgram, sizeof(UdpBlockHdr) + pay, 0);
            if (sent < 0) {
                perror("UDP send");
                free(dgram); close(sock); EVP_MD_CTX_free(sha); return -1;
            }
            if (mode == MODE_STOP_WAIT) {
                UdpAckPkt ack;
                if (recv(sock, &ack, sizeof(ack), 0) == (ssize_t)sizeof(ack) &&
                    ack.type == UDP_ACK &&
                    be64toh(ack.seq_num) == seq && ack.ok) {
                    acked = 1; break;
                }
                /* Timeout or wrong ACK – retry */
            } else {
                acked = 1; break;
            }
        }
        if (mode == MODE_STOP_WAIT && !acked)
            fprintf(stderr, "Warn: block %lu unacknowledged after %d retries\n",
                    (unsigned long)seq, retries);

        sent_bytes += pay; msgs++; seq++;
    }

    double elapsed = now_sec() - t0;
    unsigned char hash[32];
    sha256_final(sha, hash);
    print_stats("UDP", mode == MODE_STREAMING ? "streaming" : "stop-and-wait",
                elapsed, msgs, sent_bytes, hash);

    free(dgram);
    close(sock);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* QUIC helpers                                                                */
/* ═══════════════════════════════════════════════════════════════════════════ */

/* Read exactly n bytes from a QUIC stream (blocking). */
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

/* Write exactly n bytes to a QUIC stream. */
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

/* ═══════════════════════════════════════════════════════════════════════════ */
/* QUIC client                                                                 */
/* ═══════════════════════════════════════════════════════════════════════════ */
static int quic_client(const char *host, uint16_t port,
                        uint32_t bsz, uint64_t total, uint8_t mode)
{
    SSL_CTX *ctx = SSL_CTX_new(OSSL_QUIC_client_method());
    if (!ctx) { ERR_print_errors_fp(stderr); return -1; }

    /* Skip certificate verification – we use a self-signed cert on the server */
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, NULL);

    SSL *ssl = SSL_new(ctx);
    if (!ssl) { SSL_CTX_free(ctx); return -1; }

    /* ALPN: "cdp1" */
    SSL_set_alpn_protos(ssl, QUIC_ALPN_WIRE, sizeof(QUIC_ALPN_WIRE));

    /* Resolve server address */
    char port_str[8];
    snprintf(port_str, sizeof(port_str), "%u", (unsigned)port);
    BIO_ADDRINFO *res = NULL;
    if (!BIO_lookup_ex(host, port_str, BIO_LOOKUP_CLIENT,
                        AF_INET, SOCK_DGRAM, 0, &res)) {
        fprintf(stderr, "BIO_lookup_ex failed for %s:%s\n", host, port_str);
        SSL_free(ssl); SSL_CTX_free(ctx); return -1;
    }

    int sock = -1;
    BIO_ADDR *peer_addr = NULL;
    for (const BIO_ADDRINFO *ai = res; ai; ai = BIO_ADDRINFO_next(ai)) {
        sock = BIO_socket(BIO_ADDRINFO_family(ai), SOCK_DGRAM, 0, 0);
        if (sock < 0) continue;
        if (!BIO_connect(sock, BIO_ADDRINFO_address(ai), 0)) {
            BIO_closesocket(sock); sock = -1; continue;
        }
        if (!BIO_socket_nbio(sock, 1)) {
            BIO_closesocket(sock); sock = -1; continue;
        }
        peer_addr = BIO_ADDR_dup(BIO_ADDRINFO_address(ai));
        break;
    }
    BIO_ADDRINFO_free(res);

    if (sock < 0) {
        fprintf(stderr, "QUIC: cannot connect UDP socket to %s:%u\n", host, port);
        SSL_free(ssl); SSL_CTX_free(ctx); return -1;
    }

    BIO *bio = BIO_new(BIO_s_datagram());
    BIO_set_fd(bio, sock, BIO_CLOSE);
    SSL_set_bio(ssl, bio, bio);
    SSL_set1_initial_peer_addr(ssl, peer_addr);
    SSL_set_tlsext_host_name(ssl, host);
    BIO_ADDR_free(peer_addr);

    if (SSL_connect(ssl) < 1) {
        fprintf(stderr, "QUIC handshake failed\n");
        ERR_print_errors_fp(stderr);
        SSL_free(ssl); SSL_CTX_free(ctx); return -1;
    }

    /* Session header */
    SessionHdr sh;
    memcpy(sh.magic, "CDP1", 4);
    sh.mode       = mode;
    sh.block_size  = htonl(bsz);
    sh.total_bytes = htobe64(total);
    if (quic_write_all(ssl, &sh, sizeof(sh)) < 0) {
        fprintf(stderr, "QUIC: failed to send session header\n");
        SSL_free(ssl); SSL_CTX_free(ctx); return -1;
    }

    uint8_t *buf = malloc(bsz);
    if (!buf) { SSL_free(ssl); SSL_CTX_free(ctx); return -1; }
    EVP_MD_CTX *sha = sha256_init();

    double   t0 = now_sec();
    uint64_t seq = 0, sent_bytes = 0, msgs = 0;

    while (sent_bytes < total) {
        uint32_t pay = (uint32_t)((total - sent_bytes < bsz)
                                   ? (total - sent_bytes) : bsz);
        fill_data(buf, pay, sent_bytes);
        sha256_update(sha, buf, pay);

        BlockHdr bh;
        bh.seq_num      = htobe64(seq);
        bh.payload_size = htonl(pay);
        bh.is_last      = (sent_bytes + pay >= total) ? 1 : 0;

        if (quic_write_all(ssl, &bh, sizeof(bh)) < 0 ||
            quic_write_all(ssl, buf, pay) < 0) {
            fprintf(stderr, "QUIC write failed at seq %lu\n", (unsigned long)seq);
            break;
        }

        if (mode == MODE_STOP_WAIT) {
            Ack ack;
            if (quic_read_all(ssl, &ack, sizeof(ack)) < 0) {
                fprintf(stderr, "QUIC: no ACK for seq %lu\n", (unsigned long)seq);
                break;
            }
        }
        sent_bytes += pay; msgs++; seq++;
    }

    double elapsed = now_sec() - t0;
    unsigned char hash[32];
    sha256_final(sha, hash);
    print_stats("QUIC", mode == MODE_STREAMING ? "streaming" : "stop-and-wait",
                elapsed, msgs, sent_bytes, hash);

    free(buf);

    /* Conclude write side of the stream, then shut down the connection */
    SSL_stream_conclude(ssl, 0);
    while (SSL_shutdown(ssl) == 0)
        ;

    SSL_free(ssl);
    SSL_CTX_free(ctx);
    return 0;
}

/* ═══════════════════════════════════════════════════════════════════════════ */
/* main                                                                        */
/* ═══════════════════════════════════════════════════════════════════════════ */
static void usage(const char *prog)
{
    fprintf(stderr,
        "Usage: %s -p tcp|udp|quic -b BLOCK -d DATA_MB [options]\n"
        "  -p  Protocol : tcp, udp, quic\n"
        "  -b  Block size in bytes (1..65535)\n"
        "  -d  Data to transfer in MB (e.g. 500 or 1024)\n"
        "  -H  Server hostname/IP  (default: %s)\n"
        "  -P  Server port         (default: %d)\n"
        "  -m  Mode: streaming (default) or stop-wait\n"
        "\nExamples:\n"
        "  %s -p tcp  -b 1000  -d 500\n"
        "  %s -p udp  -b 65535 -d 1024 -m stop-wait\n"
        "  %s -p quic -b 10000 -d 500\n",
        prog, DEFAULT_HOST, DEFAULT_PORT, prog, prog, prog);
}

int main(int argc, char *argv[])
{
    const char *proto    = NULL;
    const char *host     = DEFAULT_HOST;
    const char *mode_str = "streaming";
    uint16_t    port     = DEFAULT_PORT;
    uint32_t    bsz      = 0;
    uint64_t    data_mb  = 0;
    int c;

    while ((c = getopt(argc, argv, "p:H:P:b:d:m:h")) != -1) {
        switch (c) {
        case 'p': proto    = optarg; break;
        case 'H': host     = optarg; break;
        case 'P': port     = (uint16_t)atoi(optarg); break;
        case 'b': bsz      = (uint32_t)atoi(optarg); break;
        case 'd': data_mb  = (uint64_t)atoll(optarg); break;
        case 'm': mode_str = optarg; break;
        default:  usage(argv[0]); return 1;
        }
    }

    if (!proto || bsz == 0 || data_mb == 0) {
        fprintf(stderr, "Error: -p, -b and -d are required.\n");
        usage(argv[0]); return 1;
    }
    if (bsz < 1 || bsz > 65535) {
        fprintf(stderr, "Block size must be 1..65535.\n"); return 1;
    }

    uint8_t  mode  = (strcmp(mode_str, "stop-wait") == 0)
                     ? MODE_STOP_WAIT : MODE_STREAMING;
    uint64_t total = data_mb * 1024ULL * 1024ULL;

    printf("Protocol : %s | Block : %u B | Data : %lu MB | Mode : %s\n",
           proto, bsz, (unsigned long)data_mb, mode_str);
    printf("Server   : %s:%u\n\n", host, port);

    if      (strcmp(proto, "tcp")  == 0) return tcp_client (host, port, bsz, total, mode);
    else if (strcmp(proto, "udp")  == 0) return udp_client (host, port, bsz, total, mode);
    else if (strcmp(proto, "quic") == 0) return quic_client(host, port, bsz, total, mode);
    else { fprintf(stderr, "Unknown protocol: %s\n", proto); return 1; }
}
