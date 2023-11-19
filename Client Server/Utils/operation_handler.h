#ifndef OPERATION_HANDLER_H
#define OPERATION_HANDLER_H

#include "server_setup.h"

#define CLIENT_INPUT_MAX_LENGTH 1000
#define MAX_WORD_SIZE 100

void operation_handler(char *nm_response, char *client_input);
int connect_to_ss(char *ip, int port);

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

void operation_handler(char *nm_response, char *client_input)
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
        // printf("Sending request to the storage server at the IP address %s and port %d\n", ip, port);

        // create a new socket for communication with the storage server
        int sock = connect_to_ss(ip, port);

        if (sock == -1) // check if connection was successful
            return;

        printf(YEL "SS < %s\n" reset, client_input);
        sendMessage(sock, client_input);

        // if the operation is reads then we have to gather the whhole response from the storage server in chunks
        if (strncmp(client_input, "READ", 4) == 0)
        {
            printf(GRN "SS > ");
            while (1)
            {
                char *SS_response = readMessage(sock);
                int resp_len = strlen(SS_response);

                if (strncmp(SS_response + resp_len - 3, "END", 3) == 0)
                {
                    SS_response[resp_len - 3] = '\0';
                    printf("%s" reset, SS_response);
                    break;
                }

                printf("%s", SS_response);
            }
            printf("\n");
            close(sock);
        }
        else //  for writing or getinfo
        {
            char *SS_response = readMessage(sock);
            printf(GRN "SS > %s\n" reset, SS_response);
            close(sock);
        }
    }

    else
    {
        printf("%s\n", nm_response); // LS will be printed here
    }
}

#endif