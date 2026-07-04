#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

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

    while(1) {
        printf("[Server] Waiting for connection...\n");
        // BLOCKING CALL
        if ((client_fd = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Handle one client fully before accepting the next
        handle_client(client_fd);
    }
    return 0;
}
