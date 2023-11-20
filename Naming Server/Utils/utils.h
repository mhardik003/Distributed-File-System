#ifndef UTILS_H

#define UTILS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdarg.h>

#define GRN "\e[0;32m"
#define MAG "\e[0;35m"
#define RED "\e[0;31m"
#define CYN "\e[0;36m"
#define reset "\e[0m"

// User defined constants and macros and global variables

#define NM_Client_PORT 8080     // port for naming server's communication with the client
#define NM_SS_PORT_LISTEN 8081  // naming server listens to this port for storage server's connection
#define NM_SS_PORT_CONNECT 8082 // naming server connects to storage servers through this port (only when NM is initiating the connection)
#define HASH_MAP_SIZE 100
#define BUFFER_RECV_SIZE 4096 // buffer size for receiving messages

int NM_client_fd;
int NM_SS_fd;

#include "hashmap.h"
#include "LRUCaching.h"
#include "SS_list.h"

typedef struct
{
    int socket_desc;
    char ip_address[INET_ADDRSTRLEN]; // ip of the client or storage server
} connection_info;

static HashmapItem *accessible_paths_hashmap[HASH_MAP_SIZE];     // stores the list of accessible paths and their corresponding values  (ip, port, num_readers, isWriting)
static HashmapItem *bkp_accessible_paths_hashmap[HASH_MAP_SIZE]; // stores the list of accessible paths of the backup naming server and their corresponding values  (ip, port, num_readers, isWriting)
static LRUCache *cache;                                           // stores the list of recently accessed paths and their corresponding values  (ip, port, num_readers, isWriting)

ListNode* head_list = NULL; // head of the list of storage servers

void custom_print(const char *format, ...) {
    va_list args;
    FILE *file = fopen("nm_logs.log", "a");

    // Print to stdout (terminal)
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    // Print to file
    va_start(args, format);
    vfprintf(file, format, args);
    va_end(args);
    fclose(file);
}

#define printf(format, ...) custom_print(format, ##__VA_ARGS__)

// User defined headers
#include "server_setup.h"
#include "operation_handler.h"
#include "client_handler.h"
#include "SS_handler.h"

#endif