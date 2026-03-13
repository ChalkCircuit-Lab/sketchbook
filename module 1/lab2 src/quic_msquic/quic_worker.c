// =============================================================================
// QUIC WORKER (Consumer)
//
// Requests and processes tasks from the QUIC Coordinator.
// Equivalent to ../worker.c (TCP).
//
// Flow per iteration:
//   1. Open QUIC connection to coordinator
//   2. Open stream, send MSG_READY, receive TaskResponse
//   3. If has_task: close connection, sleep(duration), reconnect, send MSG_DONE
//   4. If no task: close connection, wait 3s, retry
// =============================================================================

#include "quic_common.h"

const QUIC_API_TABLE* MsQuic;
HQUIC Registration;
HQUIC Configuration;

// --- Stream Callback ---
QUIC_STATUS QUIC_API
StreamCallback(HQUIC Stream, void* Context, QUIC_STREAM_EVENT* Event)
{
    SyncContext* ctx = (SyncContext*)Context;

    switch (Event->Type) {
    case QUIC_STREAM_EVENT_RECEIVE:
        ctx->recv_len = 0;
        for (uint32_t i = 0; i < Event->RECEIVE.BufferCount; i++) {
            uint32_t chunk = Event->RECEIVE.Buffers[i].Length;
            if (ctx->recv_len + chunk > sizeof(ctx->recv_buf))
                chunk = sizeof(ctx->recv_buf) - ctx->recv_len;
            memcpy(ctx->recv_buf + ctx->recv_len,
                   Event->RECEIVE.Buffers[i].Buffer, chunk);
            ctx->recv_len += chunk;
        }
        break;

    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        free(Event->SEND_COMPLETE.ClientContext);
        break;

    case QUIC_STREAM_EVENT_PEER_SEND_SHUTDOWN:
        sync_ctx_signal(ctx);
        break;

    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        if (!Event->SHUTDOWN_COMPLETE.AppCloseInProgress)
            MsQuic->StreamClose(Stream);
        break;

    default: break;
    }
    return QUIC_STATUS_SUCCESS;
}

// --- Connection Callback ---
QUIC_STATUS QUIC_API
ConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event)
{
    SyncContext* ctx = (SyncContext*)Context;

    switch (Event->Type) {
    case QUIC_CONNECTION_EVENT_CONNECTED:
        ctx->connected = TRUE;
        sync_ctx_signal(ctx);
        break;
    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        if (!Event->SHUTDOWN_COMPLETE.AppCloseInProgress)
            MsQuic->ConnectionClose(Connection);
        if (!ctx->connected) sync_ctx_signal(ctx);
        break;
    default: break;
    }
    return QUIC_STATUS_SUCCESS;
}

BOOLEAN load_client_config()
{
    QUIC_SETTINGS settings = {0};
    settings.IdleTimeoutMs = QUIC_IDLE_MS;
    settings.IsSet.IdleTimeoutMs = TRUE;

    QUIC_STATUS s;
    s = MsQuic->ConfigurationOpen(Registration, &Alpn, 1,
            &settings, sizeof(settings), NULL, &Configuration);
    if (QUIC_FAILED(s)) return FALSE;

    QUIC_CREDENTIAL_CONFIG cred = {0};
    cred.Type = QUIC_CREDENTIAL_TYPE_NONE;
    cred.Flags = QUIC_CREDENTIAL_FLAG_CLIENT
               | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION;
    s = MsQuic->ConfigurationLoadCredential(Configuration, &cred);
    return !QUIC_FAILED(s);
}

// Open a QUIC connection, send one Message, receive a response.
// Returns received length, or 0 on failure. Response is in ctx->recv_buf.
int quic_request(SyncContext* ctx, const Message* msg, int msg_len)
{
    ctx->connected = FALSE;
    ctx->recv_len = 0;
    ctx->done = 0;

    HQUIC conn;
    QUIC_STATUS s;

    s = MsQuic->ConnectionOpen(Registration, ConnectionCallback, ctx, &conn);
    if (QUIC_FAILED(s)) return 0;

    s = MsQuic->ConnectionStart(conn, Configuration,
            QUIC_ADDRESS_FAMILY_UNSPEC, "127.0.0.1", QUIC_PORT);
    if (QUIC_FAILED(s)) { MsQuic->ConnectionClose(conn); return 0; }

    sync_ctx_wait(ctx); // Wait for CONNECTED
    if (!ctx->connected) return 0;

    HQUIC stream;
    s = MsQuic->StreamOpen(conn, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, ctx, &stream);
    if (QUIC_FAILED(s)) goto done;
    s = MsQuic->StreamStart(stream, QUIC_STREAM_START_FLAG_NONE);
    if (QUIC_FAILED(s)) { MsQuic->StreamClose(stream); goto done; }

    quic_stream_send(MsQuic, stream, msg, msg_len, TRUE);
    sync_ctx_wait(ctx); // Wait for response

done:
    MsQuic->ConnectionShutdown(conn, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    usleep(50000);
    return ctx->recv_len;
}

int main()
{
    QUIC_STATUS s;
    if (QUIC_FAILED(MsQuicOpen2(&MsQuic))) { printf("MsQuicOpen2 failed\n"); return 1; }
    if (QUIC_FAILED(MsQuic->RegistrationOpen(&RegConfig, &Registration))) return 1;
    if (!load_client_config()) return 1;

    printf("=== QUIC WORKER started ===\n");

    SyncContext ctx;
    sync_ctx_init(&ctx);

    while (1) {
        // 1. Request work
        Message msg = { .type = MSG_READY };
        int rlen = quic_request(&ctx, &msg, sizeof(msg));

        if (rlen < (int)sizeof(TaskResponse)) {
            printf("[Worker] Cannot reach coordinator. Retrying in 2s...\n");
            sleep(2);
            continue;
        }

        TaskResponse resp;
        memcpy(&resp, ctx.recv_buf, sizeof(resp));

        if (!resp.has_task) {
            printf("[Worker] No tasks available. Idle for 3s...\n");
            sleep(3);
            continue;
        }

        // 2. Process the task
        printf("[Worker] Processing Task %d: '%s' (%ds)...\n",
               resp.task_id, resp.description, resp.duration);
        sleep(resp.duration);
        printf("[Worker] Task %d DONE.\n", resp.task_id);

        // 3. Report completion
        Message done = { .type = MSG_DONE, .task_id = resp.task_id };
        quic_request(&ctx, &done, sizeof(done));
    }

    sync_ctx_destroy(&ctx);
    MsQuic->ConfigurationClose(Configuration);
    MsQuic->RegistrationClose(Registration);
    MsQuicClose(MsQuic);
    return 0;
}
