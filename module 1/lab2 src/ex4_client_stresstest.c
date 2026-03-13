#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_PORT 8080
#define SERVER_IP "127.0.0.1"
#define NUM_CLIENTS 20      // How many concurrent clients to simulate

// This function acts as a single client
void* client_task(void* arg) {
    long client_id = (long)arg;
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[1024] = {0};
    char message[50];

    // 1. Create Socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("[Client %ld] Socket creation error\n", client_id);
        return NULL;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("[Client %ld] Invalid address\n", client_id);
        return NULL;
    }

    // 2. Connect to Server
    // If the server's "backlog" (listen queue) is full, this might fail or hang
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("[Client %ld] Connection Failed (Server busy?)\n", client_id);
        return NULL;
    }

    // 3. Send Data
    sprintf(message, "Hello from Client ID %ld", client_id);
    send(sock, message, strlen(message), 0);
    // printf("[Client %ld] Message sent.\n", client_id);

    // 4. Receive Response
    int valread = read(sock, buffer, 1024);
    if (valread > 0) {
        buffer[valread] = '\0';
        printf("[Client %ld] Server replied: %s\n", client_id, buffer);
    }

    // 5. Close
    close(sock);
    return NULL;
}

int main() {
    pthread_t threads[NUM_CLIENTS];
    printf("--- Starting Stress Test with %d Concurrent Clients ---\n", NUM_CLIENTS);

    // Create multiple threads to simulate simultaneous users
    for (long i = 0; i < NUM_CLIENTS; i++) {
        // We pass 'i' as the argument. Casting to void* prevents memory issues 
        // in this simple loop, though malloc is safer for complex structs.
        if (pthread_create(&threads[i], NULL, client_task, (void*)i) != 0) {
            perror("Failed to create thread");
        }
    }

    // Wait for all clients to finish
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("--- Stress Test Completed ---\n");
    return 0;
}
