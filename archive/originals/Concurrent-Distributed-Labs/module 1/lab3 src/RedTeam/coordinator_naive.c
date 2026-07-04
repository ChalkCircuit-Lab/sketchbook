#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "common.h"

// --- THREAD SAFE QUEUE (NAIVE IMPLEMENTATION) ---
#define QUEUE_SIZE 100
Message task_queue[QUEUE_SIZE];
int q_count = 0;
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;

void enqueue(Message m) {
    pthread_mutex_lock(&q_lock);
    if (q_count < QUEUE_SIZE) {
        task_queue[q_count++] = m;
        printf("[Coord] Task %d enqueued. Total: %d\n", m.id, q_count);
    }
    pthread_mutex_unlock(&q_lock);
}

// --- VULNERABILITY: Separate function with small stack frame ---
void process_task(Message* incoming, int sock) {

    Message local_copy;
    local_copy.id = incoming->id;
    local_copy.duration = incoming->duration;
    local_copy.type = incoming->type;
    // SAFE alternative: memcpy(local_copy.description, incoming->description, DESC_SIZE);
    strcpy(local_copy.description, incoming->description);

    enqueue(local_copy);
    send(sock, "ACK", 3, 0);
}

// --- NETWORK HANDLER ---
void* handle_connection(void* arg) {
    int sock = *(int*)arg;
    free(arg);
    
    // Large receive buffer on the stack
    char raw_buffer[1024]; 
    
    // 1. Receive data (No strict size check here, just a raw read)
    ssize_t bytes = recv(sock, raw_buffer, sizeof(raw_buffer), 0);
    if (bytes <= 0) { close(sock); return NULL; }

    // Cast the raw bytes to our Message struct
    Message* incoming = (Message*)raw_buffer;

    // 2. Process Client
    if (incoming->type == MSG_TASK_SUBMIT) {
        process_task(incoming, sock);
    }
    
    // (Worker logic omitted for brevity in naive version)
    
    close(sock);
    return NULL;
}

int main() {
    int server_fd, *new_sock;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int opt = 1; 
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    
    address.sin_port = htons(atoi(SERVER_PORT));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(1);
    }

    if (listen(server_fd, 10) < 0) {
        perror("Listen failed");
        exit(1);
    }

    printf(">>> NAIVE COORDINATOR listening on port %s...\n", SERVER_PORT);

    while(1) {
        new_sock = malloc(sizeof(int));
        *new_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        if (*new_sock < 0) {
            perror("Accept failed");
            free(new_sock);
            continue;
        }

        pthread_t th;
        if (pthread_create(&th, NULL, handle_connection, new_sock) != 0) {
            perror("Thread creation failed");
            free(new_sock);
        } else {
            pthread_detach(th); 
        }
    }
    return 0;
}
