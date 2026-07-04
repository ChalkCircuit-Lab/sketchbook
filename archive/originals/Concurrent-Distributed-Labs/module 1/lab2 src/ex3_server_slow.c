#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024];

    // 1. Setup Socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    printf("[Server] Slow Server listening on port %d...\n", PORT);

    // 2. Accept ONE connection
    new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    printf("[Server] Connection accepted.\n");

    // 3. Slow Read Loop
    int total_bytes = 0;
    int valread;
    
    // We intentionally read slowly to fill up buffers
    while ((valread = read(new_socket, buffer, 1024)) > 0) {
        total_bytes += valread;
        printf("[Server] Received chunk of %d bytes. Processing... (Total: %d)\n", valread, total_bytes);
        
        // SIMULATE LATENCY
        sleep(1); 
    }

    printf("[Server] Client closed connection. Finished.\n");
    return 0;
}
