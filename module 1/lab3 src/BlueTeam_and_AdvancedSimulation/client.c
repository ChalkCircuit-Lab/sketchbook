#include "common.h"
#include <time.h>
#include <stdlib.h> // Required for atoi, srand, rand

int main(int argc, char *argv[]) {
    // 1. Initialize Random Seed
    srand(time(NULL));

    // 2. Setup Socket
    int sock = 0;
    struct sockaddr_in serv_addr;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    
    serv_addr.sin_port = htons(atoi(SERVER_PORT));

    if (inet_pton(AF_INET, SERVER_IP, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported: %s \n", SERVER_IP);
        return -1;
    }

    // Set timeouts so the client doesn't hang forever if the server is overwhelmed
    struct timeval tv = {5, 0}; // 5 second timeout
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&tv, sizeof tv);

    // 3. Connect to Coordinator
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed to %s:%s. Is the Coordinator running?\n", 
               SERVER_IP, SERVER_PORT);
        return -1;
    }

    // 4. Construct the Message
    Message msg;
    msg.type = MSG_TASK_SUBMIT;
    msg.id = rand() % 9000 + 1000;      // Random ID (1000-9999)
    msg.duration = 2 + (rand() % 8);    // Random duration (2-10 seconds)

    // Handle User Input for Description (Safe Copy)
    const char* description_text = (argc > 1) ? argv[1] : "Standard Processing Task";
    
    // Use strncpy to ensure we don't overflow client-side buffer
    strncpy(msg.description, description_text, DESC_SIZE - 1);
    msg.description[DESC_SIZE - 1] = '\0'; // Enforce null-termination

    // 5. Send Task
    printf("[Client] Submitting Task ID %d: '%s' (Est. Time: %ds)\n", 
           msg.id, msg.description, msg.duration);
    
    send(sock, &msg, sizeof(msg), 0);

    // 6. Wait for ACK (Blocking)
    // 
    // Note: In a production client, we would use a timeout (select/poll) here 
    // to avoid hanging forever if the server crashes before sending ACK.
    char buffer[16] = {0};
    int valread = recv(sock, buffer, 3, 0); // Expecting "ACK"
    
    if (valread > 0) {
        printf("[Client] Server Acknowledged: %s\n", buffer);
    } else {
        printf("[Client] No ACK received (Server might be busy or crashed).\n");
    }

    // 7. Cleanup
    close(sock);
    return 0;
}
