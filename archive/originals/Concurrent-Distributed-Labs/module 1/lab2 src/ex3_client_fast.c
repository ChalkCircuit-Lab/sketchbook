#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT 8080
#define DATA_SIZE 1024 * 50 // Send 50KB

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char *message = malloc(DATA_SIZE);
    
    // Fill message with dummy data 'A'
    memset(message, 'A', DATA_SIZE);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }

    printf("[Client] Connected. Sending %d KB of data...\n", DATA_SIZE / 1024);

    // Start Timer
    clock_t start = clock();

    // SEND CALL
    // Even though the server sleeps for seconds, this will return almost instantly
    // because the OS Kernel buffers the data.
    int bytes_sent = send(sock, message, DATA_SIZE, 0);

    // Stop Timer
    clock_t end = clock();
    double time_taken = ((double)(end - start)) / CLOCKS_PER_SEC;

    printf("[Client] send() returned! Bytes sent: %d\n", bytes_sent);
    printf("[Client] Time taken to execute send(): %f seconds\n", time_taken);
    printf("[Client] Exiting immediately.\n");

    close(sock);
    return 0;
}
