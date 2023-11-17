#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>

#include "get_accessible_paths.h"
// #include "utils.h"

#define SERVER_PORT 8081

int START_PORT = 5000;
// int MAX_PATH_LENGTH = 4096;
// int MAX_TOTAL_LENGTH = 1000000;


int SS_NM_PORT;
int SS_Client_PORT;

int createServerSocket()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd <= 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

int findAvailablePort()
{
    int test_sock;
    struct sockaddr_in test_addr;
    int port = START_PORT;

    while (port < 65535)
    {
        test_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (test_sock < 0)
        {
            perror("Socket creation error in findAvailablePort");
            exit(EXIT_FAILURE);
        }

        memset(&test_addr, 0, sizeof(test_addr));
        test_addr.sin_family = AF_INET;
        test_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        test_addr.sin_port = htons(port);

        if (bind(test_sock, (struct sockaddr *)&test_addr, sizeof(test_addr)) < 0)
        {
            if (errno == EADDRINUSE)
            {
                // Port is already in use, try next port
                close(test_sock);
                port++;
            }
            else
            {
                perror("bind failed in findAvailablePort");
                close(test_sock);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // Found an available port
            START_PORT = port + 1;
            close(test_sock);
            return port;
        }
    }
    return -1; // No available port found
}

void bindServerSocket(int server_fd, struct sockaddr_in *address)
{
    if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
}

void startListening(int server_fd)
{
    if (listen(server_fd, 3) < 0)
    {
        // printf("meow");
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int acceptConnection(int server_fd, struct sockaddr_in *address, char *ip_buffer)
{
    int addrlen = sizeof(*address);
    int new_socket = accept(server_fd, (struct sockaddr *)address, (socklen_t *)&addrlen);
    if (new_socket < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    // Copy the IP address to the provided buffer
    strncpy(ip_buffer, inet_ntoa(address->sin_addr), INET_ADDRSTRLEN);
    ip_buffer[INET_ADDRSTRLEN - 1] = '\0'; // Null-terminate the string

    return new_socket;
}

void sendMessage(int socket, char *message)
{
    send(socket, message, strlen(message), 0);
    printf("Message sent\n");
}

void connectToServer(int sock, struct sockaddr_in *serv_addr)
{
    if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
    {
        perror("Connection Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void readMessage(int sock)
{
    char buffer[1024] = {0};
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

int sendInfoToNM(char *server_ip, int SS_NM_fd, struct sockaddr_in SS_NM_address)
{
    // for getting accessible paths
    char path[MAX_PATH_LENGTH];
    char *allPaths = (char *)malloc(MAX_TOTAL_LENGTH);

    char *allPathsCopy = (char *)malloc(MAX_TOTAL_LENGTH);
    char *selectedPaths = (char *)malloc(MAX_TOTAL_LENGTH);
    char *firstMessageToNM = (char *)malloc(MAX_TOTAL_LENGTH);

    allPaths[0] = '\0';
    int length = 0;

    if (getcwd(path, sizeof(path)) == NULL)
    {
        perror("getcwd() error");
        free(allPaths);
        return 1;
    }

    listFilesRecursively(path, "", &allPaths, &length); // get all paths
    strcpy(allPathsCopy, allPaths);                     // copy all paths to another string

    char *token = strtok(allPaths, "\n"); // get all paths line by line
    int index = 1;

    while (token != NULL)
    {
        printf("%d %s\n", index++, token);
        token = strtok(NULL, "\n");
    }

    strcpy(allPaths, allPathsCopy);
    getSelectedPaths(allPaths, selectedPaths);

    snprintf(firstMessageToNM, MAX_TOTAL_LENGTH, "ssinit\n%d %d\n", SS_NM_PORT, SS_Client_PORT);
    strcat(firstMessageToNM, selectedPaths);

    printf("Enter the IP address of the Naming Server: ");
    fgets(server_ip, INET_ADDRSTRLEN, stdin);

    server_ip[strcspn(server_ip, "\n")] = 0; // Remove newline character if present

    memset(&SS_NM_address, 0, sizeof(SS_NM_address));
    SS_NM_address.sin_family = AF_INET;
    SS_NM_address.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_ip, &SS_NM_address.sin_addr) <= 0)
    { // Convert IPv4 and IPv6 addresses from text to binary form
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    connectToServer(SS_NM_fd, &SS_NM_address);
    sendMessage(SS_NM_fd, firstMessageToNM);
    printf("Sent initialization message to naming server!\n");

    readMessage(SS_NM_fd); // Read confirmation message from server
}

#endif