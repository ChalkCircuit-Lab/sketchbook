// =============================================================================
// QUIC COORDINATOR (Broker)
//
// Event-driven QUIC server using msquic. Equivalent to ../coordinator.c (TCP),
// but using QUIC streams instead of TCP connections.
//
// Architecture:
//   Listener -> accepts Connections -> each Connection has Streams
//   Each stream carries one request-response exchange (SUBMIT/READY/DONE).
//
// KEY CONCEPT: Unlike TCP where we spawn a thread per connection and do
// blocking recv/send, here msquic calls our callbacks on its internal threads.
// All processing happens inside the stream callback.
// =============================================================================

#include <pthread.h>
#include "quic_common.h"

#define QUEUE_SIZE 100

// --- Global QUIC handles ---
const QUIC_API_TABLE* MsQuic;
HQUIC Registration;
HQUIC Configuration;

// --- Thread-safe Task Queue (same as TCP version) ---
typedef struct {
    Message tasks[QUEUE_SIZE];
    int head, tail, count;
    pthread_mutex_t lock;
} TaskQueue;

TaskQueue queue;

void queue_init() {
    queue.head = queue.tail = queue.count = 0;
    pthread_mutex_init(&queue.lock, NULL);
}

void enqueue(Message t) {
    pthread_mutex_lock(&queue.lock);
    if (queue.count < QUEUE_SIZE) {
        queue.tasks[queue.tail] = t;
        queue.tail = (queue.tail + 1) % QUEUE_SIZE;
        queue.count++;
        printf("[Coord] Task %d enqueued (%s). Queue: %d\n",
               t.task_id, t.description, queue.count);
    } else {
        printf("[Coord] Queue FULL! Task %d dropped.\n", t.task_id);
    }
    pthread_mutex_unlock(&queue.lock);
}

int dequeue(Message *t) {
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

// --- QUIC Stream Callback (handles one request-response) ---
// This is called by msquic when data arrives on a stream, when a send
// completes, or when the stream shuts down. This is the QUIC equivalent
// of handle_connection() in the TCP version.
QUIC_STATUS QUIC_API
StreamCallback(HQUIC Stream, void* Context, QUIC_STREAM_EVENT* Event)
{
    (void)Context;

    switch (Event->Type) {

    case QUIC_STREAM_EVENT_RECEIVE: {
        // Data received from client on this stream.
        // Collect all buffer fragments into one contiguous buffer.
        uint32_t total = 0;
        for (uint32_t i = 0; i < Event->RECEIVE.BufferCount; i++)
            total += Event->RECEIVE.Buffers[i].Length;

        if (total < sizeof(Message)) break;

        // Reconstruct the Message from the received buffers
        Message msg;
        uint8_t* dst = (uint8_t*)&msg;
        uint32_t copied = 0;
        for (uint32_t i = 0; i < Event->RECEIVE.BufferCount && copied < sizeof(Message); i++) {
            uint32_t chunk = Event->RECEIVE.Buffers[i].Length;
            if (copied + chunk > sizeof(Message)) chunk = sizeof(Message) - copied;
            memcpy(dst + copied, Event->RECEIVE.Buffers[i].Buffer, chunk);
            copied += chunk;
        }
        msg.description[DESC_SIZE - 1] = '\0';

        // Process the message (same logic as TCP version)
        switch (msg.type) {

        case MSG_SUBMIT:
            printf("[Coord] Producer submitted Task %d: '%s' (%ds)\n",
                   msg.task_id, msg.description, msg.duration);
            enqueue(msg);
            quic_stream_send(MsQuic, Stream, "ACK", 3, TRUE);
            break;

        case MSG_READY: {
            TaskResponse resp = {0};
            Message t;
            if (dequeue(&t)) {
                resp.has_task = 1;
                resp.task_id = t.task_id;
                resp.duration = t.duration;
                strncpy(resp.description, t.description, DESC_SIZE);
                printf("[Coord] Assigned Task %d to worker.\n", t.task_id);
            } else {
                resp.has_task = 0;
                printf("[Coord] No tasks. Worker goes idle.\n");
            }
            quic_stream_send(MsQuic, Stream, &resp, sizeof(resp), TRUE);
            break;
        }

        case MSG_DONE:
            printf("[Coord] Task %d completed by worker.\n", msg.task_id);
            quic_stream_send(MsQuic, Stream, "OK", 2, TRUE);
            break;

        default:
            printf("[Coord] Unknown message type %d.\n", msg.type);
        }
        break;
    }

    case QUIC_STREAM_EVENT_SEND_COMPLETE:
        // Our response was sent. Free the send buffer.
        free(Event->SEND_COMPLETE.ClientContext);
        break;

    case QUIC_STREAM_EVENT_SHUTDOWN_COMPLETE:
        MsQuic->StreamClose(Stream);
        break;

    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

// --- QUIC Connection Callback ---
QUIC_STATUS QUIC_API
ConnectionCallback(HQUIC Connection, void* Context, QUIC_CONNECTION_EVENT* Event)
{
    (void)Context;

    switch (Event->Type) {

    case QUIC_CONNECTION_EVENT_CONNECTED:
        printf("[Coord] New QUIC connection established.\n");
        // Enable session resumption (0-RTT on reconnect)
        MsQuic->ConnectionSendResumptionTicket(
            Connection, QUIC_SEND_RESUMPTION_FLAG_NONE, 0, NULL);
        break;

    case QUIC_CONNECTION_EVENT_PEER_STREAM_STARTED:
        // Client opened a new stream — set our stream callback.
        // KEY DIFFERENCE: In TCP, each connection = one request.
        // In QUIC, one connection can have many streams (multiplexing).
        MsQuic->SetCallbackHandler(
            Event->PEER_STREAM_STARTED.Stream, (void*)StreamCallback, NULL);
        break;

    case QUIC_CONNECTION_EVENT_SHUTDOWN_COMPLETE:
        printf("[Coord] Connection closed.\n");
        MsQuic->ConnectionClose(Connection);
        break;

    default:
        break;
    }
    return QUIC_STATUS_SUCCESS;
}

// --- QUIC Listener Callback ---
QUIC_STATUS QUIC_API
ListenerCallback(HQUIC Listener, void* Context, QUIC_LISTENER_EVENT* Event)
{
    (void)Listener;
    (void)Context;

    if (Event->Type == QUIC_LISTENER_EVENT_NEW_CONNECTION) {
        MsQuic->SetCallbackHandler(
            Event->NEW_CONNECTION.Connection, (void*)ConnectionCallback, NULL);
        return MsQuic->ConnectionSetConfiguration(
            Event->NEW_CONNECTION.Connection, Configuration);
    }
    return QUIC_STATUS_NOT_SUPPORTED;
}

// --- Server Setup ---
BOOLEAN load_server_config(const char* cert_file, const char* key_file)
{
    QUIC_SETTINGS settings = {0};
    settings.IdleTimeoutMs = QUIC_IDLE_MS;
    settings.IsSet.IdleTimeoutMs = TRUE;
    settings.ServerResumptionLevel = QUIC_SERVER_RESUME_AND_ZERORTT;
    settings.IsSet.ServerResumptionLevel = TRUE;
    settings.PeerBidiStreamCount = 100; // Allow many concurrent streams
    settings.IsSet.PeerBidiStreamCount = TRUE;

    QUIC_STATUS s;
    s = MsQuic->ConfigurationOpen(Registration, &Alpn, 1,
            &settings, sizeof(settings), NULL, &Configuration);
    if (QUIC_FAILED(s)) { printf("ConfigurationOpen failed: 0x%x\n", s); return FALSE; }

    QUIC_CERTIFICATE_FILE cert = { .CertificateFile = (char*)cert_file,
                                    .PrivateKeyFile  = (char*)key_file };
    QUIC_CREDENTIAL_CONFIG cred = {0};
    cred.Type = QUIC_CREDENTIAL_TYPE_CERTIFICATE_FILE;
    cred.CertificateFile = &cert;

    s = MsQuic->ConfigurationLoadCredential(Configuration, &cred);
    if (QUIC_FAILED(s)) { printf("LoadCredential failed: 0x%x\n", s); return FALSE; }

    return TRUE;
}

int main(int argc, char* argv[])
{
    const char* cert = (argc > 1) ? argv[1] : "server.cert";
    const char* key  = (argc > 2) ? argv[2] : "server.key";

    queue_init();

    QUIC_STATUS s;
    if (QUIC_FAILED(s = MsQuicOpen2(&MsQuic))) {
        printf("MsQuicOpen2 failed: 0x%x\n", s); return 1;
    }
    if (QUIC_FAILED(s = MsQuic->RegistrationOpen(&RegConfig, &Registration))) {
        printf("RegistrationOpen failed: 0x%x\n", s); return 1;
    }
    if (!load_server_config(cert, key)) return 1;

    HQUIC Listener;
    if (QUIC_FAILED(s = MsQuic->ListenerOpen(Registration, ListenerCallback, NULL, &Listener))) {
        printf("ListenerOpen failed: 0x%x\n", s); return 1;
    }

    QUIC_ADDR addr = {0};
    QuicAddrSetFamily(&addr, QUIC_ADDRESS_FAMILY_UNSPEC);
    QuicAddrSetPort(&addr, QUIC_PORT);

    if (QUIC_FAILED(s = MsQuic->ListenerStart(Listener, &Alpn, 1, &addr))) {
        printf("ListenerStart failed: 0x%x\n", s); return 1;
    }

    printf("=== QUIC COORDINATOR listening on port %d ===\n", QUIC_PORT);
    printf("    (TLS 1.3 encrypted, cert: %s)\n", cert);
    printf("    Press Enter to stop.\n\n");
    getchar();

    MsQuic->ListenerClose(Listener);
    MsQuic->ConfigurationClose(Configuration);
    MsQuic->RegistrationClose(Registration);
    MsQuicClose(MsQuic);
    return 0;
}
