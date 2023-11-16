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



// User defined constants and macros and global variables


#define NM_Client_PORT 8080 // port for naming server's communication with the client
#define NM_SS_PORT 8081 // port for naming server's communication with the storage server
#define HASH_MAP_SIZE 100
#define BUFFER_RECV_SIZE 4096 // buffer size for receiving messages


#include "hashmap.h"

typedef struct {
    int socket_desc;
    char ip_address[INET_ADDRSTRLEN]; // ip of the client or storage server
} connection_info;


static HashmapItem* accessible_paths_hashmap[HASH_MAP_SIZE];

// User defined headers
#include "server_setup.h"
#include "operation_handler.h"
#include "client_handler.h"
#include "SS_handler.h"

#endif