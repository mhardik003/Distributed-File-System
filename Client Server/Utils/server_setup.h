#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 8080
int START_PORT = 8000;
int CLIENT_NM_PORT;
int CLIENT_SS_PORT;

void print_DFS_features() {
    printf("_______________________________________________________________\n\n");
    printf("\t\t\tWelcome to the DFS!\n");
    printf("_______________________________________________________________\n\n");
    printf("You can perform the following 7 operations:\n");

    printf("1. Read a file\n");
    printf("Syntax: READ <path>\n\n");

    printf("2. Get information of a file\n");
    printf("Syntax: GETINFO <path>\n\n");
    
    printf("3. Write to a file\n");
    printf("Syntax: WRITE <path> <text>\n\n");

    printf("4. Create a directory\n");
    printf("Syntax: CREATE <path>\n\n");

    printf("5. Delete a file or directory\n");
    printf("Syntax: DELETE <path>\n\n");

    printf("6. Display all files and folders in a folder\n");
    printf("Syntax: LS <path>\n\n");

    printf("7. Copy a file or directory to another filer or directory\n");
    printf("Syntax: COPY <source path> <destination path>\n\n");
}

int findAvailablePort() {
    int test_sock;
    struct sockaddr_in test_addr;
    int port = START_PORT;

    while (port < 65535) {
        test_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (test_sock < 0) {
            perror("Socket creation error in findAvailablePort");
            exit(EXIT_FAILURE);
        }

        memset(&test_addr, 0, sizeof(test_addr));
        test_addr.sin_family = AF_INET;
        test_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        test_addr.sin_port = htons(port);

        if (bind(test_sock, (struct sockaddr *)&test_addr, sizeof(test_addr)) < 0) {
            if (errno == EADDRINUSE) {
                // Port is already in use, try next port
                close(test_sock);
                port++;
            } else {
                perror("bind failed in findAvailablePort");
                close(test_sock);
                exit(EXIT_FAILURE);
            }
        } else {
            // Found an available port
            START_PORT = port + 1;
            close(test_sock);
            return port;
        }
    }
    return -1; // No available port found
}

int createClientSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void bindClientSocket(int sock, struct sockaddr_in *cli_addr) {
    if (bind(sock, (struct sockaddr *)cli_addr, sizeof(*cli_addr)) < 0) {
        perror("bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void sendMessage(int socket, char *message) {
    send(socket, message, strlen(message), 0);
    // printf("Message sent\n");
}

char *readMessage(int sock) {
    // char buffer[1024] = {0};
    char *buffer =  (char *)malloc(1024);
    read(sock, buffer, 1024);
    return buffer;
}

void connectToServer(int sock, struct sockaddr_in *serv_addr) {
    if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

#endif