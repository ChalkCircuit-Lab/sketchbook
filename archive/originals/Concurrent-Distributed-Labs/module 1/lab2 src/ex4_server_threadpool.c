#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

#define PORT 8080
#define THREAD_POOL_SIZE 4   // Number of worker threads
#define QUEUE_SIZE 100       // Max pending clients

// --- TASK QUEUE STRUCTURE ---
typedef struct {
    int client_sockets[QUEUE_SIZE];
    int head;
    int tail;
    int count; // Current number of items in queue
    
    pthread_mutex_t lock;
    pthread_cond_t not_empty; // Condition Variable: signals when queue has data
} TaskQueue;

// Global Queue instance
TaskQueue queue;

// Initialize synchronization primitives
void init_queue() {
    queue.head = 0;
    queue.tail = 0;
    queue.count = 0;
    pthread_mutex_init(&queue.lock, NULL);
    pthread_cond_init(&queue.not_empty, NULL);
}

// PRODUCER: Main thread calls this to add a client
void enqueue(int client_socket) {
    pthread_mutex_lock(&queue.lock);
    
    if (queue.count < QUEUE_SIZE) {
        queue.client_sockets[queue.tail] = client_socket;
        queue.tail = (queue.tail + 1) % QUEUE_SIZE;
        queue.count++;
        
        // Signal a sleeping worker that there is work to do
        pthread_cond_signal(&queue.not_empty);
    } else {
        printf("[Server] Queue is full! Rejecting client %d.\n", client_socket);
        close(client_socket);
    }
    
    pthread_mutex_unlock(&queue.lock);
}

// CONSUMER: Worker threads call this to get a client
int dequeue() {
    // Note: The caller MUST hold the lock before calling this
    int client_socket = -1;
    
    if (queue.count > 0) {
        client_socket = queue.client_sockets[queue.head];
        queue.head = (queue.head + 1) % QUEUE_SIZE;
        queue.count--;
    }
    
    return client_socket;
}

// --- WORKER LOGIC ---

void handle_client(int client_socket) {
    char buffer[1024];
    // Simulate work
    printf("[Worker Thread %ld] Handling socket %d...\n", pthread_self(), client_socket);
    
    // Simple Echo
    int valread = read(client_socket, buffer, 1024);
    if (valread > 0) {
        send(client_socket, "Processed by ThreadPool\n", 24, 0);
    }
    
    close(client_socket);
}

void* worker_thread(void* arg) {
    while (1) {
        int client_socket;

        // 1. Lock mutex to access the queue safely
        pthread_mutex_lock(&queue.lock);

        // 2. WAIT while queue is empty
        // pthread_cond_wait automatically unlocks the mutex and puts thread to sleep.
        // When signaled, it wakes up and re-locks the mutex.
        while (queue.count == 0) {
            pthread_cond_wait(&queue.not_empty, &queue.lock);
        }

        // 3. Retrieve task
        client_socket = dequeue();

        // 4. Unlock mutex so other threads can access the queue
        pthread_mutex_unlock(&queue.lock);

        // 5. Process the client (OUTSIDE the lock for parallelism)
        if (client_socket != -1) {
            handle_client(client_socket);
        }
    }
    return NULL;
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    pthread_t thread_pool[THREAD_POOL_SIZE];

    // 1. Initialize Queue
    init_queue();
    
    // 2. Create the Pool of Workers
    for (int i = 0; i < THREAD_POOL_SIZE; i++) {
        pthread_create(&thread_pool[i], NULL, worker_thread, NULL);
    }

    // 3. Socket Setup (Standard TCP boilerplate)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed"); exit(EXIT_FAILURE);
    }
    
    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed"); exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 20) < 0) {
        perror("Listen failed"); exit(EXIT_FAILURE);
    }

    printf("[Main] Thread Pool Server started on port %d with %d threads.\n", PORT, THREAD_POOL_SIZE);

    // 4. Main Accept Loop
    while (1) {
        client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }
        
        // Add to queue (Non-blocking for the main thread unless queue is full)
        enqueue(client_socket);
    }

    return 0;
}
