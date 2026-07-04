// =============================================================================
// EXTENSION B: DoS MITIGATION (Rate Limiting)
// =============================================================================
//
// WHAT THIS FIXES vs. the original lab doc:
//   1. Per-IP tracking using a simple fixed-size table (not just a global counter).
//   2. Placed BEFORE conn_enqueue() so it works with the thread pool model.
//      (If placed after, the main thread blocks on conn_enqueue and never
//      reaches the rate-limit check when the pool is saturated.)
//   3. Extracts client IP correctly from sockaddr_storage.
//
// HOW TO PLUG IN:
//   - Add the rate limiter struct and functions below to coordinator_secure.c
//   - Replace the accept loop in main() with the rate-limited version
// =============================================================================

// ---- coordinator_secure.c: Add after includes ----

#include <time.h>

#define RATE_LIMIT      10     // Max requests per second per IP
#define RATE_TABLE_SIZE 64     // Track up to 64 unique IPs

typedef struct {
    char ip[INET6_ADDRSTRLEN];
    int  count;
    time_t window_start;
} RateEntry;

RateEntry rate_table[RATE_TABLE_SIZE];
int rate_table_count = 0;
// Note: accessed only from the single accept-loop thread, no lock needed.

// Find or create a rate entry for this IP. Returns 1 if allowed, 0 if blocked.
int rate_limit_check(const char* ip) {
    time_t now = time(NULL);

    // Search for existing entry
    for (int i = 0; i < rate_table_count; i++) {
        if (strcmp(rate_table[i].ip, ip) == 0) {
            // Reset window if a new second has started
            if (now != rate_table[i].window_start) {
                rate_table[i].window_start = now;
                rate_table[i].count = 0;
            }
            rate_table[i].count++;
            return rate_table[i].count <= RATE_LIMIT;
        }
    }

    // New IP — register it
    if (rate_table_count < RATE_TABLE_SIZE) {
        strncpy(rate_table[rate_table_count].ip, ip, INET6_ADDRSTRLEN - 1);
        rate_table[rate_table_count].ip[INET6_ADDRSTRLEN - 1] = '\0';
        rate_table[rate_table_count].window_start = now;
        rate_table[rate_table_count].count = 1;
        rate_table_count++;
    }
    return 1; // Allow (first request)
}

// Extract a printable IP string from sockaddr_storage
void get_client_ip(struct sockaddr_storage* addr, char* buf, size_t buflen) {
    if (addr->ss_family == AF_INET) {
        struct sockaddr_in* v4 = (struct sockaddr_in*)addr;
        inet_ntop(AF_INET, &v4->sin_addr, buf, buflen);
    } else {
        struct sockaddr_in6* v6 = (struct sockaddr_in6*)addr;
        inet_ntop(AF_INET6, &v6->sin6_addr, buf, buflen);
    }
}

// ---- coordinator_secure.c: Replace the accept loop in main() ----

/*
    // Accept loop with rate limiting
    while (1) {
        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int new_sock = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size);

        if (new_sock < 0) continue;

        // Extract client IP
        char client_ip[INET6_ADDRSTRLEN];
        get_client_ip(&client_addr, client_ip, sizeof(client_ip));

        // Rate limit check — BEFORE conn_enqueue (critical for thread pool)
        if (!rate_limit_check(client_ip)) {
            printf("[Firewall] Rate limit exceeded from %s! Dropping.\n", client_ip);
            close(new_sock);
            continue; // Don't waste a pool thread on this
        }

        conn_enqueue(new_sock);
    }
*/

// ---- Testing ----
// 1. Recompile coordinator_secure with the rate limiter.
// 2. Run dos_flooder — it sends many rapid connections from the same IP.
// 3. Expected: coordinator logs "[Firewall] Rate limit exceeded from 127.0.0.1!"
//    and only processes 10 connections per second. CPU stays low.
// 4. Run client normally — it sends 1 request and gets through fine.
