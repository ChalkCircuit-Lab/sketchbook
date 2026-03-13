// =============================================================================
// EXTENSION C: GRACEFUL SHUTDOWN (Signal Handling)
// =============================================================================
//
// WHAT THIS FIXES vs. the original lab doc:
//   1. Does NOT call printf/close/exit inside the signal handler.
//      Those functions are not async-signal-safe and cause undefined behavior.
//   2. Instead, sets a volatile flag. The main loop checks it and exits cleanly.
//   3. Uses sigaction() instead of signal() for portable behavior.
//   4. Cleans up both task_queue and conn_queue resources.
//
// HOW TO PLUG IN:
//   - Add the code below to coordinator_secure.c (locations marked)
//   - Make server_fd global (move out of main)
// =============================================================================

// ---- coordinator_secure.c: Add after includes ----

#include <signal.h>

// Global flag — only type safe to modify in a signal handler
volatile sig_atomic_t shutdown_requested = 0;

// Global server_fd so the cleanup function can close it
int server_fd;

void sigint_handler(int sig) {
    // ONLY set the flag — do NOT call printf, close, exit, etc.
    shutdown_requested = 1;
}

void install_signal_handler() {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}

void cleanup_and_exit() {
    printf("\n[System] Shutting down gracefully...\n");

    // 1. Close server socket — stops accepting new connections
    close(server_fd);

    // 2. Destroy task queue resources
    pthread_mutex_destroy(&task_queue.lock);

    // 3. Destroy connection queue resources
    pthread_mutex_destroy(&conn_queue.lock);
    pthread_cond_destroy(&conn_queue.not_empty);
    pthread_cond_destroy(&conn_queue.not_full);

    // 4. If Extension A is active, also destroy pending_lock:
    // pthread_mutex_destroy(&pending_lock);

    printf("[System] Cleanup complete. Goodbye.\n");
    exit(0);
}

// ---- coordinator_secure.c: Update main() ----

// 1. Remove "int server_fd;" from main (it's now global)
// 2. Add before the accept loop:

/*
    install_signal_handler();
*/

// 3. Replace the accept loop with:

/*
    while (!shutdown_requested) {
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        if (new_sock < 0) {
            if (shutdown_requested) break; // accept() interrupted by signal
            continue;
        }

        conn_enqueue(new_sock);
    }

    cleanup_and_exit();
*/

// ---- Testing ----
// 1. Start coordinator_secure.
// 2. Submit a few tasks with ./client.
// 3. Press Ctrl+C.
// 4. Expected: "[System] Shutting down gracefully..." instead of abrupt termination.
