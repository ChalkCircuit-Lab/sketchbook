// threads_pthreads.c
// gcc -std=c11 -O2 -Wall -Wextra -pthread threads_pthreads.c -o threads_pthreads
//./threads_pthreads > result
#define _POSIX_C_SOURCE 200809L
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 30
#define RUNS 10

static pthread_barrier_t start_barrier;

static void* thread_func(void* arg) {
    int id = *(int*)arg;

    // All threads wait here, then are released together -> scheduler race is visible
    pthread_barrier_wait(&start_barrier);

    printf("[%d] started execution.\n", id);

    // Same for all threads: small amount of identical work to encourage preemption
    volatile unsigned long long x = 0;
    for (unsigned long long i = 0; i < 5000000ULL; ++i) {
        x += i;
    }

    printf("[%d] finished.\n", id);
    return NULL;
}

int main(void) {
    for (int run = 1; run <= RUNS; ++run) {
        printf("=== Run %d ===\n", run);

        pthread_t threads[NUM_THREADS];
        int ids[NUM_THREADS];

        // +1 includes main thread so it can release them all at once
        if (pthread_barrier_init(&start_barrier, NULL, NUM_THREADS + 1) != 0) {
            perror("pthread_barrier_init");
            return 1;
        }

        for (int i = 0; i < NUM_THREADS; ++i) {
            ids[i] = i; // unique IDs 0..29

            int rc = pthread_create(&threads[i], NULL, thread_func, &ids[i]);
            if (rc != 0) {
                fprintf(stderr, "pthread_create failed for thread %d (rc=%d)\n", i, rc);
                return 1;
            }
        }

        // Release all worker threads simultaneously
        pthread_barrier_wait(&start_barrier);

        for (int i = 0; i < NUM_THREADS; ++i) {
            int rc = pthread_join(threads[i], NULL);
            if (rc != 0) {
                fprintf(stderr, "pthread_join failed for thread %d (rc=%d)\n", i, rc);
                return 1;
            }
        }

        pthread_barrier_destroy(&start_barrier);
        printf("\n");
    }

    return 0;
}
