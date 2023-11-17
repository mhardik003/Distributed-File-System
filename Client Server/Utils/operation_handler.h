#ifndef OPERATION_HANDLER_H
#define OPERATION_HANDLER_H

#include "server_setup.h"

#define CLIENT_INPUT_MAX_LENGTH 1000
#define MAX_WORD_SIZE 100

void operation_handler(char *nm_response, char **client_input_tokens, int num_tokens);
int connect_to_ss(char *ip, int port);
void readFile(int sock, char **client_input_tokens);
void writeFile(int sock, char **client_input_tokens, int num_tokens);
void getInfo(int sock, char **client_input_tokens);
void parseInput(char *nm_response, char *client_input);

int connect_to_ss(char *ip, int port)
{
    int sock;
    struct sockaddr_in serv_addr, cli_addr;
    char server_ip[INET_ADDRSTRLEN]; // buffer for the IP address

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(CLIENT_SS_PORT);

    sock = createClientSocket();
    bindClientSocket(sock, &cli_addr);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    connectToServer(sock, &serv_addr);

    return sock;
}

void parseInput(char *nm_response, char *client_input)
{
    char **tokens = (char **)malloc(CLIENT_INPUT_MAX_LENGTH * sizeof(char *));

    for (int i = 0; i < CLIENT_INPUT_MAX_LENGTH; i++)
    {
        tokens[i] = (char *)malloc(MAX_WORD_SIZE * sizeof(char));
    }
    int num_tokens = 0;
    char *token = strtok(client_input, " ");
    while (token != NULL)
    {
        strcpy(tokens[num_tokens++], token);
        token = strtok(NULL, " ");
    }

    operation_handler(nm_response, tokens, num_tokens);
}

void operation_handler(char *nm_response, char **client_input_tokens, int num_tokens)
{

    if (strncmp(nm_response, "lookup response", 15) == 0)
    {
        char *token = strtok(nm_response, "\n");
        token = strtok(NULL, "\n");
        char ip[100];
        sscanf(token, "ip:%s", ip);
        token = strtok(NULL, "\n");
        int port;
        sscanf(token, "client_port:%d", &port);
        // printf("IP is: %s\n", ip);
        // printf("Port is: %d\n", port);
        printf("Sending request to the storage server at the IP address %s and port %d\n", ip, port);

        // create a new socket for communication with the storage server
        int sock = connect_to_ss(ip, port);

        if (sock == -1) // check if connection was successful
            return;

        if (strcmp(client_input_tokens[0], "READ") == 0)
        {
            readFile(sock, client_input_tokens);
        }
        else if (strcmp(client_input_tokens[0], "WRITE") == 0)
        {
            writeFile(sock, client_input_tokens, num_tokens);
        }
        else if (strcmp(client_input_tokens[0], "GETINFO") == 0)
        {
            getInfo(sock, client_input_tokens);
        }
        else
        {
            printf("Invalid operation name\n");
        }
        char *response = readMessage(sock);
        printf("%s\n", response);
    }

    else
    {
        printf("%s\n", nm_response);
    }
}

void readFile(int sock, char **client_input_tokens)
{
    char *fileName = client_input_tokens[1];
    char *message = (char *)malloc(10000 * sizeof(char));
    strcpy(message, "Send me the contents of the file ");
    strcat(message, fileName);

    sendMessage(sock, message);
    char *SS_response = readMessage(sock);
    printf("> SS Response : %s\n", SS_response);
    close(sock);
}

void writeFile(int sock, char **client_input_tokens, int num_tokens)
{
    char *content = (char *)malloc(10000 * sizeof(char));
    for (int i = 2; i < num_tokens; i++)
    {
        strcat(content, client_input_tokens[i]);
        if (i != num_tokens - 1)
        {
            strcat(content, " ");
        }
    }

    char *fileName = client_input_tokens[1];

    char *message = (char *)malloc(10000 * sizeof(char));
    strcpy(message, "Write the following content to the file ");
    strcat(message, fileName);
    strcat(message, " : ");
    strcat(message, content);

    sendMessage(sock, message);
    char *SS_response = readMessage(sock);
    printf("> SS Response : %s\n", SS_response);
    close(sock);
}
void getInfo(int sock, char **client_input_tokens)
{
    char *fileName = client_input_tokens[1];
    char *message = (char *)malloc(10000 * sizeof(char));
    strcpy(message, "Send me the info of the file ");
    strcat(message, fileName);

    sendMessage(sock, message);
    char *SS_response = readMessage(sock);
    printf("> SS Response : %s\n", SS_response);
    close(sock);
}

#endif