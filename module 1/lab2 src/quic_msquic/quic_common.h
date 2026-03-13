#ifndef QUIC_COMMON_H
#define QUIC_COMMON_H

// =============================================================================
// Shared definitions for the QUIC Coordinator-Producer-Worker system.
//
// KEY DIFFERENCES vs. the TCP version (../common.h):
//   - Uses msquic library (event-driven callbacks vs. blocking recv/send)
//   - TLS 1.3 encryption is mandatory (QUIC requirement)
//   - Data travels over QUIC streams within a connection
//   - One QUIC connection can carry multiple streams (multiplexing)
//
// SETUP:
//   sudo apt install libmsquic
//   (or build from https://github.com/microsoft/msquic)
// =============================================================================

#include "msquic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

// --- CONFIG ---
#define QUIC_PORT       4567
#define QUIC_IDLE_MS    5000  // Connection idle timeout (ms)
#define QUIC_ALPN       "lab3"

// --- PROTOCOL (same as TCP version) ---
#define MSG_SUBMIT  1
#define MSG_READY   2
#define MSG_DONE    3

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

// --- SYNCHRONIZATION CONTEXT ---
// msquic is callback-based. To do sequential logic (send -> wait for response),
// we pass this context to callbacks. The main thread waits on the condition
// variable; callbacks signal it when data arrives or the operation completes.
typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t  cv;
    int             done;       // Flag: operation complete
    uint8_t         recv_buf[512];
    uint32_t        recv_len;
    BOOLEAN         connected;
    HQUIC           stream;     // Set by callbacks for the caller
} SyncContext;

static inline void sync_ctx_init(SyncContext* ctx) {
    memset(ctx, 0, sizeof(*ctx));
    pthread_mutex_init(&ctx->lock, NULL);
    pthread_cond_init(&ctx->cv, NULL);
}

static inline void sync_ctx_destroy(SyncContext* ctx) {
    pthread_mutex_destroy(&ctx->lock);
    pthread_cond_destroy(&ctx->cv);
}

static inline void sync_ctx_signal(SyncContext* ctx) {
    pthread_mutex_lock(&ctx->lock);
    ctx->done = 1;
    pthread_cond_signal(&ctx->cv);
    pthread_mutex_unlock(&ctx->lock);
}

static inline void sync_ctx_wait(SyncContext* ctx) {
    pthread_mutex_lock(&ctx->lock);
    while (!ctx->done) {
        pthread_cond_wait(&ctx->cv, &ctx->lock);
    }
    ctx->done = 0; // Reset for reuse
    pthread_mutex_unlock(&ctx->lock);
}

// --- QUIC HELPERS ---
static const QUIC_REGISTRATION_CONFIG RegConfig = {
    "lab3-app", QUIC_EXECUTION_PROFILE_LOW_LATENCY
};

static const QUIC_BUFFER Alpn = {
    sizeof(QUIC_ALPN) - 1, (uint8_t*)QUIC_ALPN
};

// Send a buffer on a QUIC stream (with FIN = last message on this stream)
static inline QUIC_STATUS quic_stream_send(
    const QUIC_API_TABLE* msquic, HQUIC stream,
    const void* data, uint32_t len, BOOLEAN fin)
{
    void* raw = malloc(sizeof(QUIC_BUFFER) + len);
    if (!raw) return QUIC_STATUS_OUT_OF_MEMORY;

    QUIC_BUFFER* buf = (QUIC_BUFFER*)raw;
    buf->Buffer = (uint8_t*)raw + sizeof(QUIC_BUFFER);
    buf->Length = len;
    memcpy(buf->Buffer, data, len);

    QUIC_SEND_FLAGS flags = fin ? QUIC_SEND_FLAG_FIN : QUIC_SEND_FLAG_NONE;
    QUIC_STATUS status = msquic->StreamSend(stream, buf, 1, flags, raw);
    if (QUIC_FAILED(status)) {
        free(raw);
    }
    return status;
}

#endif
