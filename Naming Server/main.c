#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SERVER_PORT 8080

int createServerSocket() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    return server_fd;
}

void bindServerSocket(int server_fd, struct sockaddr_in *address) {
    if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
}

void startListening(int server_fd) {
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int acceptConnection(int server_fd, struct sockaddr_in *address) {
    int addrlen = sizeof(*address);
    int new_socket = accept(server_fd, (struct sockaddr *)address, (socklen_t *)&addrlen);
    if (new_socket < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

void sendMessage(int socket, char *message) {
    send(socket, message, strlen(message), 0);
    printf("Message sent!\n");
}

void readMessage(int sock) {
    char buffer[1024] = {0};
    read(sock, buffer, 1024);
    printf("%s", buffer);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(SERVER_PORT);

    server_fd = createServerSocket();
    bindServerSocket(server_fd, &address);
    startListening(server_fd);

    while(1) {
        new_socket = acceptConnection(server_fd, &address);
        while(1) {
            readMessage(new_socket);
            char *confirmationMsg = "Displayed!";
            sendMessage(new_socket, confirmationMsg);
        }
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
