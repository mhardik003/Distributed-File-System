#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT 8080
#define BUFFER_SIZE 1024

// Function to display error message and exit the program
void handle_error(const char *error_msg) {
    printf("\033[1;31mERROR: %s\n\033[0m", error_msg);
    exit(EXIT_FAILURE);
}

// Function to retrieve the IP address of the host machine
char* get_host_ip() {
    char host_buffer[256];
    struct hostent *host_info;
    
    if (gethostname(host_buffer, sizeof(host_buffer)) != 0) 
        handle_error("Error in obtaining IP address");

    host_info = gethostbyname(host_buffer);
    if (host_info == NULL) 
        handle_error("Error in obtaining IP address");

    return inet_ntoa(*((struct in_addr*)host_info->h_addr_list[0]));
}

// Function to create a socket and return its file descriptor
int create_server_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) 
        handle_error("Socket creation failed");
    return socket_fd;
}

// Function to bind the socket to the server address
void bind_server_socket(int socket_fd, struct sockaddr_in *server_addr) {
    if (bind(socket_fd, (struct sockaddr*)server_addr, sizeof(*server_addr)) == -1)
        handle_error("Couldn't bind socket");
}

// Function to listen for incoming connections
void listen_for_connections(int socket_fd) {
    if (listen(socket_fd, 5) == -1)
        handle_error("Listening failed");
}

// Function to accept a client connection
int accept_client_connection(int socket_fd, struct sockaddr_in *client_addr, socklen_t *addr_size) {
    int connection_fd = accept(socket_fd, (struct sockaddr*)client_addr, addr_size);
    if (connection_fd < 0)
        handle_error("Couldn't establish connection");
    return connection_fd;
}

// Function to receive a message from the client
void receive_client_message(int connection_fd, char* buffer) {
    if (recv(connection_fd, buffer, BUFFER_SIZE, 0) < 0)
        handle_error("Nothing was received from client");
}

// Function to send a message to the client
void send_server_message(int connection_fd, const char* message) {
    if (send(connection_fd, message, strlen(message), 0) == -1)
        handle_error("Couldn't send message to client");
}

int main() {
    char* ip_address = get_host_ip();
    int socket_fd = create_server_socket();
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size = sizeof(client_addr);
    char client_response[BUFFER_SIZE];

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    bind_server_socket(socket_fd, &server_addr);
    listen_for_connections(socket_fd);

    int connection_fd = accept_client_connection(socket_fd, &client_addr, &addr_size);
    receive_client_message(connection_fd, client_response);

    printf("Received from client: %s\n", client_response);

    // char* server_msg = "Hello Client! Server has received your message.\0";
    char server_msg[256] = "\0";
    strcpy(server_msg, "Hello Client! Server has received your message.\0");
    send_server_message(connection_fd, server_msg);

    close(connection_fd);
    close(socket_fd);
    return 0;
}
yo0 