#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "utils.h"

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

    ssize_t bytes_read = read(socket, buffer, 1023); // Read one less to leave room for null terminator

    if (bytes_read < 0)
    {
        // Handle read error
        free(buffer);
        close(socket);
        return NULL;
    }

    buffer[bytes_read] = '\0'; // Manually null-terminate the string

    printf("> Received the following message from the client : %s\n", buffer);
    parseClientInput(buffer, socket);
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
        new_socket = acceptConnection(client_fd, &client_address, ip_buffer);

        if (new_socket < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        else
        {
            printf("Client connected with IP address %s and port %d\n", ip_buffer, ntohs(client_address.sin_port));
            connection_info *info = (connection_info *)malloc(sizeof(connection_info));
            info->socket_desc = new_socket;
            strcpy(info->ip_address, ip_buffer);
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handleClientConnection, (void *)info);
        }
    }
}

#endif