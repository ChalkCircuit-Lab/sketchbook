// =============================================================================
// QUIC PRODUCER (Client)
//
// Submits tasks to the QUIC Coordinator. Equivalent to ../producer.c (TCP).
//
// KEY CONCEPT: Because msquic is callback-based (not blocking like TCP's
// send/recv), we use a SyncContext with mutex+condvar to wait for responses.
// The flow is: open connection -> wait for CONNECTED callback ->
// open stream -> send Message -> wait for RECEIVE callback -> done.
// =============================================================================

#include <time.h>
#include "quic_common.h"

const QUIC_API_TABLE* MsQuic;
HQUIC Registration;
HQUIC Configuration;

// --- Client Stream Callback ---
QUIC_STATUS QUIC_API
StreamCallback(HQUIC Stream, void* Context, QUIC_STREAM_EVENT* Event)
{
    SyncContext* ctx = (SyncContext*)Context;

    switch (Event->Type) {
    case QUIC_STREAM_EVENT_RECEIVE:
        // Response received from coordinator
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
        // Coordinator finished sending — wake up the main thread
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

// --- Client Connection Callback ---
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
        if (!ctx->connected) sync_ctx_signal(ctx); // Unblock on failure
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
               | QUIC_CREDENTIAL_FLAG_NO_CERTIFICATE_VALIDATION; // Self-signed OK
    s = MsQuic->ConfigurationLoadCredential(Configuration, &cred);
    return !QUIC_FAILED(s);
}

// Submit one task via QUIC. Returns 1 on success.
int submit_task(int id, int duration, const char* desc)
{
    SyncContext ctx;
    sync_ctx_init(&ctx);

    HQUIC conn;
    QUIC_STATUS s;

    // 1. Open and start connection
    s = MsQuic->ConnectionOpen(Registration, ConnectionCallback, &ctx, &conn);
    if (QUIC_FAILED(s)) { sync_ctx_destroy(&ctx); return 0; }

    s = MsQuic->ConnectionStart(conn, Configuration,
            QUIC_ADDRESS_FAMILY_UNSPEC, "127.0.0.1", QUIC_PORT);
    if (QUIC_FAILED(s)) { MsQuic->ConnectionClose(conn); sync_ctx_destroy(&ctx); return 0; }

    sync_ctx_wait(&ctx); // Wait for CONNECTED
    if (!ctx.connected) { sync_ctx_destroy(&ctx); return 0; }

    // 2. Open a stream and send the SUBMIT message
    HQUIC stream;
    s = MsQuic->StreamOpen(conn, QUIC_STREAM_OPEN_FLAG_NONE, StreamCallback, &ctx, &stream);
    if (QUIC_FAILED(s)) goto done;
    s = MsQuic->StreamStart(stream, QUIC_STREAM_START_FLAG_NONE);
    if (QUIC_FAILED(s)) { MsQuic->StreamClose(stream); goto done; }

    Message msg = { .type = MSG_SUBMIT, .task_id = id, .duration = duration };
    strncpy(msg.description, desc, DESC_SIZE - 1);
    msg.description[DESC_SIZE - 1] = '\0';
    quic_stream_send(MsQuic, stream, &msg, sizeof(msg), TRUE);

    // 3. Wait for response (ACK)
    sync_ctx_wait(&ctx);

    if (ctx.recv_len >= 3 && memcmp(ctx.recv_buf, "ACK", 3) == 0) {
        printf("[Producer] Task %d accepted: '%s' (%ds)\n", id, desc, duration);
    } else {
        printf("[Producer] Task %d: no ACK received.\n", id);
    }

done:
    MsQuic->ConnectionShutdown(conn, QUIC_CONNECTION_SHUTDOWN_FLAG_NONE, 0);
    usleep(100000); // Brief pause for clean shutdown
    sync_ctx_destroy(&ctx);
    return 1;
}

int main(int argc, char* argv[])
{
    srand(time(NULL));

    QUIC_STATUS s;
    if (QUIC_FAILED(MsQuicOpen2(&MsQuic))) { printf("MsQuicOpen2 failed\n"); return 1; }
    if (QUIC_FAILED(MsQuic->RegistrationOpen(&RegConfig, &Registration))) return 1;
    if (!load_client_config()) return 1;

    if (argc > 1) {
        int count = atoi(argv[1]);
        if (count <= 0) count = 5;
        printf("[Producer] Auto-generating %d tasks over QUIC...\n", count);
        for (int i = 1; i <= count; i++) {
            char desc[DESC_SIZE];
            snprintf(desc, DESC_SIZE, "QUIC-Task-%d", i);
            submit_task(i, 2 + rand() % 6, desc);
        }
    } else {
        printf("[Producer] Interactive mode (QUIC). Type description or 'quit':\n");
        int id = 1;
        char line[256];
        while (1) {
            printf("> "); fflush(stdout);
            if (!fgets(line, sizeof(line), stdin)) break;
            line[strcspn(line, "\n")] = '\0';
            if (strcmp(line, "quit") == 0 || strcmp(line, "q") == 0) break;
            const char* desc = strlen(line) > 0 ? line : "Default Task";
            submit_task(id++, 2 + rand() % 6, desc);
        }
    }

    MsQuic->ConfigurationClose(Configuration);
    MsQuic->RegistrationClose(Registration);
    MsQuicClose(MsQuic);
    printf("[Producer] Done.\n");
    return 0;
}
