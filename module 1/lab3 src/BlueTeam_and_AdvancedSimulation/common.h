#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h> 

// INFRASTRUCTURE CONFIG
#define SERVER_PORT "9000"
#define SERVER_HOST "coordinator"   // For Docker DNS (Worker/Flooder)
#define SERVER_IP   "127.0.0.1"   // For Static IP (Legacy Client)

// MESSAGE CODES
#define MSG_TASK_SUBMIT 1
#define MSG_WORKER_READY 2

// VULNERABILITY CONTEXT: Fixed buffer size
#define DESC_SIZE 64 

typedef struct {
    int type;           // 1 = Client, 2 = Worker
    int id;             // Task ID
    int duration;       // Simulated processing time
    char description[DESC_SIZE]; 
} Message;

typedef struct {
    int has_task;       // 1 = Yes, 0 = No
    int task_id;
    int duration;
    char description[DESC_SIZE];
} TaskResponse;

// Ensures we read exactly 'len' bytes. Handles fragmentation/partial packets.
ssize_t recv_all(int sock, void *buffer, size_t len) {
    size_t bytes_read = 0;
    char *ptr = (char *)buffer;
    while (bytes_read < len) {
        ssize_t chunk = recv(sock, ptr + bytes_read, len - bytes_read, 0);
        if (chunk <= 0) return chunk; // Error or Connection Closed
        bytes_read += chunk;
    }
    return bytes_read;
}

#endif