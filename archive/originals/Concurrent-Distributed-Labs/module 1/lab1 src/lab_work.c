#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

// Account Structure
typedef struct {
    int id;             // Unique identifier for sorting locks
    int balance;
    pthread_mutex_t lock;
} Account;

Account accA, accB;

// The SAFE Transfer Function
void transfer_safe(Account *from, Account *to, int amount, const char* thread_name) {
    printf("[%s] Initiating transfer: %d -> %d (Amount: %d)\n", thread_name, from->id, to->id, amount);

    // --- STEP 1: Determine Locking Order ---
    // We enforce a hierarchy: Always lock the smaller ID first.
    Account *first_lock, *second_lock;

    if (from->id < to->id) {
        first_lock = from;
        second_lock = to;
    } else {
        first_lock = to;
        second_lock = from;
    }

    // --- STEP 2: Acquire Locks in Order ---
    pthread_mutex_lock(&first_lock->lock);
    printf("[%s] Locked first mutex (Account ID: %d).\n", thread_name, first_lock->id);
    
    // We keep the sleep to prove the solution works even with delays/context switches
    sleep(1); 

    pthread_mutex_lock(&second_lock->lock);
    printf("[%s] Locked second mutex (Account ID: %d). Acquired both resources!\n", thread_name, second_lock->id);

    // --- STEP 3: Critical Section (Actual Transfer) ---
    if (from->balance >= amount) {
        from->balance -= amount;
        to->balance += amount;
        printf("[%s] SUCCESS: New Balances -> A: %d, B: %d\n", thread_name, accA.balance, accB.balance);
    } else {
        printf("[%s] FAILED: Insufficient funds.\n", thread_name);
    }

    // --- STEP 4: Release Locks ---
    // (Reverse order is good practice, though not strictly required for correctness here)
    pthread_mutex_unlock(&second_lock->lock);
    pthread_mutex_unlock(&first_lock->lock);
    
    printf("[%s] Transaction finished.\n", thread_name);
}

void* thread_func_1(void* arg) {
    // Thread 1 wants A -> B (ID 1 -> ID 2)
    // It will lock 1, then 2.
    transfer_safe(&accA, &accB, 100, "Thread 1");
    return NULL;
}

void* thread_func_2(void* arg) {
    // Thread 2 wants B -> A (ID 2 -> ID 1)
    // Thanks to our logic, it will attempt to lock A (ID 1) FIRST, not B!
    // Finding A locked by Thread 1, it waits at the ENTRY.
    // It does not hoard the lock for B, preventing the deadlock.
    transfer_safe(&accB, &accA, 100, "Thread 2");
    return NULL;
}

int main() {
    // Initialization
    accA.id = 1; accA.balance = 1000; pthread_mutex_init(&accA.lock, NULL);
    accB.id = 2; accB.balance = 1000; pthread_mutex_init(&accB.lock, NULL);

    pthread_t t1, t2;

    printf("--- Start Safe Transfer Simulation ---\n");

    // Launch threads
    pthread_create(&t1, NULL, thread_func_1, NULL);
    pthread_create(&t2, NULL, thread_func_2, NULL);

    // Wait for completion
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    // Cleanup
    pthread_mutex_destroy(&accA.lock);
    pthread_mutex_destroy(&accB.lock);

    printf("--- Final: Program finished successfully (No Deadlock) ---\n");
    return 0;
}