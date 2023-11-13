#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define SERVER_PORT 8080
#define CLIENT_PORT 8081

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
    int sock;
    struct sockaddr_in serv_addr, cli_addr;

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(CLIENT_PORT);

    sock = createClientSocket();
    bindClientSocket(sock, &cli_addr);

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    connectToServer(sock, &serv_addr);

    while(1) {
        char message[1024];
        printf("Enter your message: ");
        fgets(message, 1024, stdin);
        sendMessage(sock, message);
        readMessage(sock); // Read confirmation message from server
    }

    close(sock);
    return 0;
}
