#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    printf("[Server] Handling client on socket %d...\n", client_socket);

    // Simulate heavy work (blocking the process)
    sleep(5); 

    int bytes_read = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';
        printf("[Server] Received: %s\n", buffer);
        send(client_socket, "ACK\n", 4, 0);
    }
    
    close(client_socket);
    printf("[Server] Client disconnected.\n");
}

// Wrapper struct to pass arguments to thread
// (In C, we must ensure the socket variable isn't overwritten before the thread starts)
void* thread_function(void* arg) {
    int client_fd = *(int*)arg;
    free(arg); // Free the memory allocated in main
    
    handle_client(client_fd); // Reuse the logic from Ex 1
    return NULL;
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // 1. Create Socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // 2. Bind
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // 3. Listen
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("[Server] Listening on port %d (Iterative Mode)...\n", PORT);

// Inside main() loop:
while(1) {
    int* client_fd_ptr = malloc(sizeof(int)); // Allocate memory for each client
    *client_fd_ptr = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    
    if (*client_fd_ptr < 0) {
        perror("Accept failed");
        free(client_fd_ptr);
        continue;
    }

    pthread_t thread_id;
    // Create a thread for this specific client
    if (pthread_create(&thread_id, NULL, thread_function, client_fd_ptr) != 0) {
        perror("Thread creation failed");
    }
    
    // Detach so resources are freed automatically when thread ends
    pthread_detach(thread_id);
}

    return 0;
}
