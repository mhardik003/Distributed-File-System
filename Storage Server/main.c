#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>

#include "Utils/get_accessible_paths.h"

#define CYN "\e[0;36m"
#define reset "\e[0m"

#define SERVER_PORT 8081
int START_PORT = 5000;
int SS_NM_PORT;
int SS_Client_PORT;

typedef struct
{
    int socket_desc;
    char ip_address[INET_ADDRSTRLEN]; // ip of the client or storage server
} connection_info;

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

int createServerSocket()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    return server_fd;
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

int acceptConnection(int server_fd, struct sockaddr_in *address,
                     char *ip_buffer)
{
    int addrlen = sizeof(*address);
    int new_socket =
        accept(server_fd, (struct sockaddr *)address, (socklen_t *)&addrlen);
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

void *handleClientConnection(void *arg)
{
    connection_info *info = (connection_info *)arg;
    int socket = info->socket_desc;
    char ip_address[INET_ADDRSTRLEN];
    strcpy(ip_address, info->ip_address);

    free(info);

    char buffer[1024] = {0};
    read(socket, buffer, 1024);
    // parseClientInput(buffer);
    // printf("%s\n", buffer);
    printf("Received the following message from the client : %s\n", buffer);
    send(socket, "Hello from the storage server", strlen("Hello from the storage server"), 0);
    printf("Hello message sent\n");
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
            connection_info *info = malloc(sizeof(connection_info));
            info->socket_desc = new_socket;
            strcpy(info->ip_address, ip_buffer);
            pthread_t client_thread;
            pthread_create(&client_thread, NULL, handleClientConnection, (void *)info);
        }
    }
}

// void *listenFromNameServer(int *NM_SS_fd)
// {
//     int ss_fd = *NM_SS_fd;
//     int new_socket;
//     struct sockaddr_in ss_address;
//     int addrlen = sizeof(ss_address);

//     while (1)
//     {
//         if ((new_socket = accept(ss_fd, (struct sockaddr *)&ss_address, (socklen_t *)&addrlen)) < 0)
//         {
//             perror("accept");
//             exit(EXIT_FAILURE);
//         }
//         printf("Storage Server connected!\n");
//         char buffer[1024] = {0};
//         read(new_socket, buffer, 1024);
//         printf("%s\n", buffer);
//         send(new_socket, "Hello from server", strlen("Hello from server"), 0);
//         printf("Hello message sent\n");
//     }
// }

int main()
{

    SS_NM_PORT = findAvailablePort();
    SS_Client_PORT = findAvailablePort();

    if (SS_NM_PORT == -1 || SS_Client_PORT == -1)
    {
        printf("No available port found. Exiting.\n");
        return -1;
    }

    printf("\nStorage Server Port for communication with naming server: %d\n", SS_NM_PORT);
    printf("Storage Server Port for communication with client: %d\n\n", SS_Client_PORT);

    int SS_client_fd, SS_NM_fd;
    struct sockaddr_in SS_client_address, SS_NM_address;

    char server_ip[INET_ADDRSTRLEN]; // Buffer for the IP address

    SS_client_address.sin_family = AF_INET;
    SS_client_address.sin_addr.s_addr = INADDR_ANY;
    SS_client_address.sin_port = htons(SS_Client_PORT);

    // SS_NM_address.sin_family = AF_INET;
    // SS_NM_address.sin_addr.s_addr = INADDR_ANY;
    // SS_NM_address.sin_port = htons(SS_NM_PORT);

    SS_NM_address.sin_family = AF_INET;
    SS_NM_address.sin_addr.s_addr = htonl(INADDR_ANY);
    SS_NM_address.sin_port = htons(SS_NM_PORT);

    SS_NM_fd = createServerSocket();
    SS_client_fd = createServerSocket();

    bindServerSocket(SS_NM_fd, &SS_NM_address);
    bindServerSocket(SS_client_fd, &SS_client_address);

    // for getting accessible paths
    char path[MAX_PATH_LENGTH];
    char *allPaths = malloc(MAX_TOTAL_LENGTH);
    char *allPathsCopy = malloc(MAX_TOTAL_LENGTH);
    char *selectedPaths = malloc(MAX_TOTAL_LENGTH);
    char *firstMessageToNM = malloc(MAX_TOTAL_LENGTH);

    allPaths[0] = '\0';
    int length = 0;

    if (getcwd(path, sizeof(path)) == NULL)
    {
        perror("getcwd() error");
        free(allPaths);
        return 1;
    }

    listFilesRecursively(path, "", &allPaths, &length);
    strcpy(allPathsCopy, allPaths);

    char *token = strtok(allPaths, "\n");
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
    fgets(server_ip, sizeof(server_ip), stdin);
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

    // startListening(SS_client_fd);
    printf("billu bageele\n");
    // startListening(SS_NM_fd);

    pthread_t client_thread, nm_thread;

    startListening(SS_client_fd);
    pthread_create(&client_thread, NULL, listenForClients, (void *)&SS_client_fd);
    // pthread_create(&nm_thread, NULL, listenFromNameServer, (void *)&SS_NM_fd);

    pthread_join(client_thread, NULL);
    // while(1) {
    // char message[1024];
    // printf(CYN"Enter Message: "reset);
    // fgets(message, 1024, stdin);
    // message[strcspn(message, "\n")] = 0; // Remove newline character if present
    // sendMessage(sock, message);
    // readMessage(sock); // Read confirmation message from server
    // }

    close(SS_client_fd);
    close(SS_NM_fd);
    return 0;
}
