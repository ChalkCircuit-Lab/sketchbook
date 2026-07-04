#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_ITERATIONS 1000000

// Shared Resource
// 'volatile' prevents the compiler from optimizing away memory access
volatile long counter = 0;

// Synchronization Mechanism
pthread_mutex_t lock;

// Flag to enable/disable protection
int use_mutex = 0;

void* increment(void* arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        if (use_mutex) {
            // --- PROTECTED CRITICAL SECTION ---
            pthread_mutex_lock(&lock);
            counter++;
            pthread_mutex_unlock(&lock);
        } else {
            // --- RACE CONDITION ---
            // This operation is NOT atomic!
            // It consists of LOAD -> ADD -> STORE
            counter++; 
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    pthread_t t1, t2;

    // Check command line args to enable mutex
    if (use_mutex) {
        printf("Mode: SAFE (Mutex Enabled)\n");
    } else {
        printf("Mode: UNSAFE (Race Condition Possible)\n");
    }

    pthread_mutex_init(&lock, NULL);

    // Create threads
    if (pthread_create(&t1, NULL, increment, NULL) != 0) {
        perror("Failed to create thread");
        return 1;
    }
    if (pthread_create(&t2, NULL, increment, NULL) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    // Wait for threads to finish
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    pthread_mutex_destroy(&lock);

    printf("Final Result:   %ld\n", counter);
    printf("Expected Result: %d\n", NUM_ITERATIONS * 2);

    if (counter != NUM_ITERATIONS * 2) {
        printf("CONCLUSION: Data Corruption Detected!\n");
    } else {
        printf("CONCLUSION: Execution Successful.\n");
    }

    return 0;
}
