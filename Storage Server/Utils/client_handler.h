#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "utils.h"

#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define reset "\e[0m"

void *handleClientConnection(void *arg)
{
    connection_info *info = (connection_info *)arg;
    int socket = info->socket_desc;
    char ip_address[INET_ADDRSTRLEN];
    strcpy(ip_address, info->ip_address);
    free(info);

    // char buffer[1024] = {0};
    char *buffer = (char *)malloc(1024 * sizeof(char));
    if (buffer == NULL)
    {
        // Handle malloc failure
        close(socket);
        return NULL;
    }
    memset(buffer, 0, 1024); // Initialize buffer to zero

    strcpy(buffer, readMessage(socket));

    printf(GRN "Client > %s\n" reset, buffer);
    parseInput(buffer, socket);
    free(buffer);
    close(socket);
}

void *handleNMConnection(void *arg)
{
    connection_info *info = (connection_info *)arg;
    int socket = info->socket_desc;
    char ip_address[INET_ADDRSTRLEN];
    strcpy(ip_address, info->ip_address);
    free(info);

    // char buffer[1024] = {0};
    char *buffer = (char *)malloc(1024 * sizeof(char));
    if (buffer == NULL)
    {
        // Handle malloc failure
        close(socket);
        return NULL;
    }
    memset(buffer, 0, 1024); // Initialize buffer to zero

    strcpy(buffer, readMessage(socket));

    printf(GRN "NM > %s\n" reset, buffer);
    parseInput(buffer, socket);
    free(buffer);
    close(socket);
}

void *listenForClients(void *NM_client_fd)
{
    int client_fd = *(int *)NM_client_fd;
    int new_socket;

    while (1)
    {
        struct sockaddr_in client_address;
        char ip_buffer[INET_ADDRSTRLEN];
        printf(YEL "Listening to requests from clients ...\n" reset);
        new_socket = acceptConnection(client_fd, &client_address, ip_buffer);

        if (new_socket < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        else
        {
            printf(GRN "Client connected with IP address %s and port %d\n" reset, ip_buffer, ntohs(client_address.sin_port));
            connection_info *info = (connection_info *)malloc(sizeof(connection_info));
            info->socket_desc = new_socket;
            strcpy(info->ip_address, ip_buffer);
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handleClientConnection, (void *)info);
        }
    }
}

void *listenFromNameServer(void *NM_SS_fd)
{
    int NM_SS_socket = *(int *)NM_SS_fd;
    int new_socket;

    while (1)
    {
        struct sockaddr_in nm_address;
        char ip_buffer[INET_ADDRSTRLEN];
        printf(YEL "Listening for request from Name Server...\n" reset);
        new_socket = acceptConnection(NM_SS_socket, &nm_address, ip_buffer);

        if (new_socket < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        else
        {
            printf(CYN "Name Server connected with IP address %s and port %d\n" reset, ip_buffer, ntohs(nm_address.sin_port));
            connection_info *info = (connection_info *)malloc(sizeof(connection_info));
            info->socket_desc = new_socket;
            strcpy(info->ip_address, ip_buffer);
            pthread_t nm_thread;
            pthread_create(&nm_thread, NULL, handleNMConnection, (void *)info);
        }
    }
}

#endif