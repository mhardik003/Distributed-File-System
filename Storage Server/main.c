#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "Utils/get_accessible_paths.h"

#define CYN "\e[0;36m"
#define reset "\e[0m"

#define SERVER_PORT 8081
int START_PORT = 5000;
int SS_NM_PORT;
int SS_Client_PORT;

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

int createSSSocket() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE);
    }
    return sock;
}

void bindSSSocket(int sock, struct sockaddr_in *ss_addr) {
    if (bind(sock, (struct sockaddr *)ss_addr, sizeof(*ss_addr)) < 0) {
        perror("bind failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void sendMessage(int socket, char *message) {
    send(socket, message, strlen(message), 0);
    printf("Message sent\n");
}

void connectToServer(int sock, struct sockaddr_in *serv_addr) {
    if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0) {
        perror("Connection Failed");
        close(sock);
        exit(EXIT_FAILURE);
    }
}

void readMessage(int sock) {
    char buffer[1024] = {0};
    read(sock, buffer, 1024);
    printf("%s\n", buffer);
}

int main() {
    SS_NM_PORT = findAvailablePort();
    SS_Client_PORT = findAvailablePort();
    if (SS_NM_PORT == -1 || SS_Client_PORT == -1) {
        printf("No available port found. Exiting.\n");
        return -1;
    }

    printf("\nStorage Server Port for communication with naming server: %d\n", SS_NM_PORT);
    printf("Storage Server Port for communication with client: %d\n\n", SS_Client_PORT);
    int sock;
    struct sockaddr_in serv_addr, ss_addr;
    char server_ip[INET_ADDRSTRLEN]; // Buffer for the IP address

    ss_addr.sin_family = AF_INET;
    ss_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    ss_addr.sin_port = htons(SS_NM_PORT);

    sock = createSSSocket();
    bindSSSocket(sock, &ss_addr);

    // for getting accessible paths
    char path[MAX_PATH_LENGTH];
    char *allPaths = malloc(MAX_TOTAL_LENGTH);
    char *allPathsCopy = malloc(MAX_TOTAL_LENGTH);
    char *selectedPaths = malloc(MAX_TOTAL_LENGTH);
    char *firstMessageToNM = malloc(MAX_TOTAL_LENGTH);

    allPaths[0] = '\0';
    int length = 0;

    if (getcwd(path, sizeof(path)) == NULL) {
        perror("getcwd() error");
        free(allPaths);
        return 1;
    }

    listFilesRecursively(path, &allPaths, &length);
    strcpy(allPathsCopy, allPaths);

    char *token = strtok(allPaths, "\n");
    int index = 1;
    while (token != NULL) {
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

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    connectToServer(sock, &serv_addr);
    sendMessage(sock, firstMessageToNM);
    printf("Sent initialization message to naming server!\n");
    readMessage(sock); // Read confirmation message from server

    // while(1) {
        // char message[1024];
        // printf(CYN"Enter Message: "reset);
        // fgets(message, 1024, stdin);
        // message[strcspn(message, "\n")] = 0; // Remove newline character if present
        // sendMessage(sock, message);
        // readMessage(sock); // Read confirmation message from server
    // }

    close(sock);
    return 0;
}

