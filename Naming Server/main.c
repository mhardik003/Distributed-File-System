#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"
#include "Utils/hashmap.h"
#include <pthread.h>

typedef struct {
    int socket_desc;
    char ip_address[INET_ADDRSTRLEN]; // ip of the client or storage server
} connection_info;

#define NM_Client_PORT 8080 // port for naming server's communication with the client
#define NM_SS_PORT 8081 // port for naming server's communication with the storage server
hashmap *accessible_paths_ip_lookup;

int createServerSocket()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

void bindServerSocket(int server_fd, struct sockaddr_in *address)
{
    if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void startListening(int server_fd)
{
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int acceptConnection(int server_fd, struct sockaddr_in *address, char *ip_buffer) {
    int addrlen = sizeof(*address);
    int new_socket = accept(server_fd, (struct sockaddr *)address, (socklen_t *)&addrlen);
    if (new_socket < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    // Copy the IP address to the provided buffer
    strncpy(ip_buffer, inet_ntoa(address->sin_addr), INET_ADDRSTRLEN);
    ip_buffer[INET_ADDRSTRLEN - 1] = '\0';  // Null-terminate the string

    return new_socket;
}

void sendMessage(int socket, char *message)
{
    send(socket, message, strlen(message), 0);
    printf("Message sent!\n");
}

char *parseInput(char *input)
{
    // array to store tokens
    char **tokens = malloc(10 * sizeof(char *));

    for (int i = 0; i < 10; i++)
    {
        tokens[i] = malloc(100 * sizeof(char));
    }
    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL)
    {
        // printf("%s\n", token);
        // store token in tokens array
        tokens[i++] = token;
        token = strtok(NULL, " ");
    }
    return operation_handler(tokens, i);
}

int parse_ssinit(char *init_message, char *ip_address)
{
    char *token;
    int num1, num2;

    token = strtok(init_message, "\n");
    if (token == NULL || strcmp(token, "ssinit") != 0) {
        return 0;
    }

    token = strtok(NULL, "\n");
    if (token == NULL || sscanf(token, "%d %d", &num1, &num2) != 2) {
        return 0;
    }

    token = strtok(NULL, "\n");
    while (token != NULL) {
        hashmap_insert(accessible_paths_ip_lookup, token, ip_address);
        token = strtok(NULL, "\n");
    }

    return 1;
}

char *readClientMessage(int sock)
{
    // printf("Now reading the message\n");
    char buffer[1024] = {0};
    if (recv(sock, buffer, 1024, 0) < 0)
    {
        return "Error in receiving the message";
    }
    printf("Message received by Client : %s", buffer);
    return parseInput(buffer);
}

char *readStorageServerMessage(int sock, char *ip_address)
{
    // printf("Now reading the message\n");
    char buffer[1024] = {0};
    if (recv(sock, buffer, 1024, 0) < 0)
    {
        return "Error in receiving the message";
    }
    printf("Message received by SS : %s", buffer);
    if(strncmp(buffer, "ssinit", 6) == 0) {
        parse_ssinit(buffer, ip_address);
        return "ssinit done";
    }
    return "Hey SS, how are you? -NM";
}

void *handleClientConnection(void *arg) {
    connection_info *info = (connection_info *)arg;
    int sock = info->socket_desc;
    char ip_buffer[INET_ADDRSTRLEN];  // Buffer to store the IP address
    strncpy(ip_buffer, info->ip_address, INET_ADDRSTRLEN);

    free(info); // Free the allocated memory for the connection info

    while (1) {
        char *message = readClientMessage(sock);
        if (message == NULL || strcmp(message, "QUIT") == 0) {
            printf("Storage Server [%s] disconnected\n", ip_buffer);
            break;
        }
        sendMessage(sock, message);
    }

    close(sock);
    return NULL;
}

void *handleStorageServerConnection(void *arg) {
    connection_info *info = (connection_info *)arg;
    int sock = info->socket_desc;
    char ip_buffer[INET_ADDRSTRLEN];
    strncpy(ip_buffer, info->ip_address, INET_ADDRSTRLEN);

    free(info);  // Free the allocated memory for the connection info

    // while (1) {
        char *message = readStorageServerMessage(sock, ip_buffer);
        if (message == NULL || strcmp(message, "QUIT") == 0) {
            printf("Storage Server [%s] disconnected\n", ip_buffer);
            close(sock);
            return NULL;
            // break;
        }
        sendMessage(sock, message);
    // }

    close(sock);
    return NULL;
}


void *listenForClients(void *arg) {
    int server_fd = *(int *)arg;
    while (1) {
        struct sockaddr_in address;
        char ip_buffer[INET_ADDRSTRLEN];  // Buffer to store the IP address

        int *new_sock = malloc(sizeof(int));
        *new_sock = acceptConnection(server_fd, &address, ip_buffer);
        printf("Connected to client %s\n", ip_buffer);

        if (*new_sock >= 0) {
            pthread_t thread_id;
            connection_info *info = malloc(sizeof(connection_info));
            info->socket_desc = *new_sock;
            strncpy(info->ip_address, ip_buffer, INET_ADDRSTRLEN);
            pthread_create(&thread_id, NULL, handleClientConnection, (void*) info);
        }
    }
    return NULL;
}

void *listenForStorageServers(void *arg) {
    int server_fd = *(int *)arg;
    while (1) {
        struct sockaddr_in address;
        char ip_buffer[INET_ADDRSTRLEN];  // Buffer to store the IP address

        int *new_sock = malloc(sizeof(int));
        *new_sock = acceptConnection(server_fd, &address, ip_buffer);
        printf("Connected to storage server %s\n", ip_buffer);

        if (*new_sock >= 0) {
            pthread_t thread_id;
            connection_info *info = malloc(sizeof(connection_info));
            info->socket_desc = *new_sock;
            strncpy(info->ip_address, ip_buffer, INET_ADDRSTRLEN);
            
            pthread_create(&thread_id, NULL, handleStorageServerConnection, (void*) info);
        }
    }
    return NULL;
}

int main() {
    int NM_client_fd, NM_SS_fd;
    struct sockaddr_in NM_client_address, NM_SS_address;

    // Address setup for both ports
    NM_client_address.sin_family = AF_INET;
    NM_client_address.sin_addr.s_addr = INADDR_ANY;
    NM_client_address.sin_port = htons(NM_Client_PORT);

    NM_SS_address.sin_family = AF_INET;
    NM_SS_address.sin_addr.s_addr = INADDR_ANY;
    NM_SS_address.sin_port = htons(NM_SS_PORT);

    // Hashmap creation
    accessible_paths_ip_lookup = create_hashmap(1000);

    // Socket creation and binding
    NM_client_fd = createServerSocket();
    NM_SS_fd = createServerSocket();

    bindServerSocket(NM_client_fd, &NM_client_address);
    bindServerSocket(NM_SS_fd, &NM_SS_address);

    // Start listening on both ports
    startListening(NM_client_fd);
    startListening(NM_SS_fd);

    pthread_t client_thread, ss_thread;

    pthread_create(&client_thread, NULL, listenForClients, (void *)&NM_client_fd);
    pthread_create(&ss_thread, NULL, listenForStorageServers, (void *)&NM_SS_fd);

    // Join threads (optional, depending on your shutdown strategy)
    pthread_join(client_thread, NULL);
    pthread_join(ss_thread, NULL);

    // Cleanup
    close(NM_client_fd);
    close(NM_SS_fd);
    return 0;
}
