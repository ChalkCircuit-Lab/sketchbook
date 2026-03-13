/*
 * common.h – Shared definitions for Homework 1
 * PCD / MISS1202O2 – Concurrent and Distributed Programming
 *
 * Wire protocol
 * ─────────────
 * TCP / QUIC (stream):
 *   Client→Server:  SessionHdr  (once, at start)
 *   Client→Server:  BlockHdr + payload  (repeated)
 *   Server→Client:  Ack  (stop-and-wait only, one per block)
 *
 * UDP (datagrams):
 *   Client→Server:  UdpSessionPkt  (first datagram)
 *   Server→Client:  UdpAckPkt      (seq=UINT64_MAX – session confirm)
 *   Client→Server:  UdpBlockHdr + payload  (data datagrams)
 *   Server→Client:  UdpAckPkt  (stop-and-wait only, one per block)
 *
 * Every multi-byte integer on the wire is big-endian (network byte order).
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <errno.h>
#include <endian.h>          /* htobe64 / be64toh (Linux) */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <openssl/evp.h>

/* ── defaults ─────────────────────────────────────────────────────────────── */
#define DEFAULT_PORT  9999
#define DEFAULT_HOST  "127.0.0.1"

/* ── transfer modes ──────────────────────────────────────────────────────── */
#define MODE_STREAMING  0
#define MODE_STOP_WAIT  1

/* ── TCP / QUIC stream protocol structs ──────────────────────────────────── */

/* Sent by the client once, at the start of each transfer session. */
typedef struct __attribute__((packed)) {
    char     magic[4];      /* "CDP1"                            */
    uint8_t  mode;          /* MODE_STREAMING / MODE_STOP_WAIT   */
    uint32_t block_size;    /* bytes per block  (network order)  */
    uint64_t total_bytes;   /* total payload    (network order)  */
} SessionHdr;               /* 17 bytes                          */

/* Prepended to each payload chunk. */
typedef struct __attribute__((packed)) {
    uint64_t seq_num;       /* 0-based sequence  (network order) */
    uint32_t payload_size;  /* actual bytes that follow           */
    uint8_t  is_last;       /* 1 for the final block             */
} BlockHdr;                 /* 13 bytes                          */

/* Sent by server in stop-and-wait mode. */
typedef struct __attribute__((packed)) {
    uint64_t seq_num;       /* echoes block seq_num              */
    uint8_t  ok;            /* 1 = received correctly            */
} Ack;                      /* 9 bytes                           */

/* ── UDP datagram protocol structs ──────────────────────────────────────── */
#define UDP_SESSION  0x01
#define UDP_BLOCK    0x02
#define UDP_ACK      0x03

/* Maximum UDP data payload per datagram.
 * IPv4: 65535 (max IP) - 20 (IP hdr) - 8 (UDP hdr) = 65507 bytes total.
 * We reserve sizeof(UdpBlockHdr)=14 bytes for our own block header. */
#define UDP_MAX_BLOCK  (65507 - 14)   /* 65493 bytes */

typedef struct __attribute__((packed)) {
    uint8_t  type;          /* UDP_SESSION                       */
    uint8_t  mode;
    uint32_t block_size;
    uint64_t total_bytes;
} UdpSessionPkt;            /* 14 bytes */

typedef struct __attribute__((packed)) {
    uint8_t  type;          /* UDP_BLOCK                         */
    uint64_t seq_num;
    uint32_t payload_size;
    uint8_t  is_last;
} UdpBlockHdr;              /* 14 bytes */

typedef struct __attribute__((packed)) {
    uint8_t  type;          /* UDP_ACK                           */
    uint64_t seq_num;       /* UINT64_MAX = session-start ACK    */
    uint8_t  ok;
} UdpAckPkt;                /* 10 bytes */

/* ── QUIC ALPN ───────────────────────────────────────────────────────────── */
/* Wire format: length byte followed by protocol name "cdp1". */
static const unsigned char QUIC_ALPN_WIRE[] = { 4, 'c', 'd', 'p', '1' };

/* ── SHA-256 helpers (OpenSSL EVP) ──────────────────────────────────────── */
static inline EVP_MD_CTX *sha256_init(void)
{
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx && EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return NULL;
    }
    return ctx;
}

static inline void sha256_update(EVP_MD_CTX *ctx, const void *data, size_t len)
{
    EVP_DigestUpdate(ctx, data, len);
}

static inline void sha256_final(EVP_MD_CTX *ctx, unsigned char out[32])
{
    unsigned int len = 32;
    EVP_DigestFinal_ex(ctx, out, &len);
    EVP_MD_CTX_free(ctx);
}

static inline void print_hash(const char *label, const unsigned char h[32])
{
    printf("%s: ", label);
    for (int i = 0; i < 32; i++) printf("%02x", h[i]);
    printf("\n");
}

/* ── monotonic timer ─────────────────────────────────────────────────────── */
static inline double now_sec(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

/* ── TCP / stream helpers ────────────────────────────────────────────────── */
/* Send exactly n bytes; returns 0 on success, -1 on error. */
static inline int send_all(int fd, const void *buf, size_t n)
{
    const char *p = (const char *)buf;
    while (n > 0) {
        ssize_t r = send(fd, p, n, 0);
        if (r <= 0) return -1;
        p += r; n -= (size_t)r;
    }
    return 0;
}

/* Receive exactly n bytes; returns 0 on success, -1 on error/EOF. */
static inline int recv_all(int fd, void *buf, size_t n)
{
    char *p = (char *)buf;
    while (n > 0) {
        ssize_t r = recv(fd, p, n, 0);
        if (r <= 0) return -1;
        p += r; n -= (size_t)r;
    }
    return 0;
}

/* ── data generation ─────────────────────────────────────────────────────── */
/* Fill buf[0..n-1] with a repeating 0x00-0xFF pattern starting at offset. */
static inline void fill_data(uint8_t *buf, uint32_t n, uint64_t offset)
{
    for (uint32_t i = 0; i < n; i++)
        buf[i] = (uint8_t)((offset + i) & 0xFF);
}

#endif /* COMMON_H */
