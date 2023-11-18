#ifndef UTILS_H
#define UTILS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

// User defined headers
#include "server_setup.h"
#include "get_accessible_paths.h"
#include "client_operation_handler.h"

// server_setup.h
int createServerSocket();
int findAvailablePort();
void bindServerSocket(int server_fd, struct sockaddr_in *address);
void startListening(int server_fd);
int acceptConnection(int server_fd, struct sockaddr_in *address, char *ip_buffer);
void sendMessage(int socket, char *message);
void connectToServer(int sock, struct sockaddr_in *serv_addr);
char *readMessage(int sock);
void sendInfoToNM();

// get_accessible_paths.h
void listFilesRecursively(char *basePath, char *currentPath, char **paths, int *length);
void getSelectedPaths(char *paths, char *selectedPaths);

// User defined structs and Global Variables

#define CYN "\e[0;36m"
#define reset "\e[0m"

// #define SERVER_PORT 8081

extern int START_PORT;



// extern int SS_NM_PORT;

// extern int SS_Client_PORT;
// extern int MAX_PATH_LENGTH;
// extern int MAX_TOTAL_LENGTH;mihika.txtmihika.txt

typedef struct
{
    int socket_desc;
    char ip_address[INET_ADDRSTRLEN]; // ip of the client or storage server
} connection_info;

#endif