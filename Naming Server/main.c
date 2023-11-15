#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "utils.h"
#include "Utils/hashmap.h"

#define NM_Client_PORT 8080 // port for naming server's communication with the client
#define NM_SS_PORT 8081 // port for naming server's communication with the storage server

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

int acceptConnection(int server_fd, struct sockaddr_in *address)
{
    int addrlen = sizeof(*address);
    int new_socket = accept(server_fd, (struct sockaddr *)address, (socklen_t *)&addrlen);
    if (new_socket < 0)
    {
        perror("accept");
        exit(EXIT_FAILURE);
    }
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

char *readMessage(int sock)
{
    // printf("Now reading the message\n");
    char buffer[1024] = {0};
    if (recv(sock, buffer, 1024, 0) < 0)
    {
        return "Error in receiving the message";
    }
    printf("Message received : %s", buffer);
    return parseInput(buffer);
}

int main()
{
    int NM_client_fd, NM_SS_fd, new_socket;
    struct sockaddr_in NM_client_address;
    struct sockaddr_in NM_SS_address;

    NM_client_address.sin_family = AF_INET;
    NM_client_address.sin_addr.s_addr = INADDR_ANY;
    NM_client_address.sin_port = htons(NM_Client_PORT);

    NM_SS_address.sin_family = AF_INET;
    NM_SS_address.sin_addr.s_addr = INADDR_ANY;
    NM_SS_address.sin_port = htons(NM_SS_PORT);

    // Create Lookup Hashmap for the accessible paths
    hashmap *accesible_paths_ip_lookup = create_hashmap(1000);

    NM_client_fd = createServerSocket();
    NM_SS_fd = createServerSocket();

    bindServerSocket(NM_client_fd, &NM_client_address);
    bindServerSocket(NM_SS_fd, &NM_SS_address);

    startListening(NM_client_fd);
    startListening(NM_SS_fd);

    while (1)
    {
        new_socket = acceptConnection(NM_SS_fd, &NM_SS_address);
        while (1)
        {
            char response[256];
            strcpy(response, readMessage(new_socket));
            printf("Sending the response as %s\n", response);
            sendMessage(new_socket, response);
        }
        close(new_socket);
    }

    close(NM_SS_fd);
    close(NM_SS_fd);
    return 0;
}
