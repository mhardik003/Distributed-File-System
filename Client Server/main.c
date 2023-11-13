#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_PORT 8080 // Port number of the naming server
#define CLIENT_PORT 8050 // Port number of the client
#define BUFFER_LENGTH 1024  // Maximum length of the message

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
int initialize_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) 
        handle_error("Socket creation failed");
    return socket_fd;
}

// Function to bind the socket to the server address
void bind_server_socket(int socket_fd, struct sockaddr_in *server_addr) {
    if (bind(socket_fd, (struct sockaddr*)server_addr, sizeof(*server_addr)) == -1)
        printf("Couldn't bind socket");      
}

// Function to listen for incoming connections
void listen_for_connections(int socket_fd) {
    if (listen(socket_fd, 5) == -1)
        handle_error("Listening failed");
}


// Function to connect to the server using the provided socket file descriptor and server address
void establish_connection(int socket_fd, struct sockaddr_in server_addr) {
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
        handle_error("Connection to server failed");
}

// Function to accept a client connection
int accept_client_connection(int socket_fd, struct sockaddr_in *client_addr, socklen_t *addr_size) {
    int connection_fd = accept(socket_fd, (struct sockaddr*)client_addr, addr_size);
    if (connection_fd < 0)
        handle_error("Couldn't establish connection");
    return connection_fd;
}

// Function to send a message to the server
void transmit_message(int socket_fd, const char* message) {
    if (send(socket_fd, message, strlen(message), 0) == -1) 
        handle_error("Message transmission to server failed");
}

// Function to receive a message from the server
void fetch_message(int socket_fd, char* server_reply) {
    if (recv(socket_fd, server_reply, BUFFER_LENGTH, 0) < 0) 
        handle_error("No data received from server");
}

int main() {
    char* ip_address = get_host_ip();
    printf("Client IP %s\n", ip_address);

    int socket_fd = initialize_socket();
    struct sockaddr_in client_addr;
    memset(&client_addr, '\0', sizeof(client_addr)); // Zero out the struct
    client_addr.sin_family = AF_INET; // IPv4 address family
    client_addr.sin_addr.s_addr = inet_addr(ip_address); // Convert IP address from text to binary form
    client_addr.sin_port = htons(CLIENT_PORT); // Convert port number from host byte order to network byte order    

    // bind the client socket to the client address
    if (bind(socket_fd, (struct sockaddr*)&client_addr, sizeof(client_addr)) == -1)
        handle_error("Couldn't bind socket");

    // set up server address to connect to
    struct sockaddr_in server_addr;
    memset(&server_addr, '\0', sizeof(server_addr)); // Zero out the struct
    server_addr.sin_family = AF_INET; // IPv4 address family
    server_addr.sin_addr.s_addr = inet_addr(ip_address); // Convert IP address from text to binary form
    server_addr.sin_port = htons(SERVER_PORT); // Convert port number from host byte order to network byte order

    printf("Connecting to naming server...\n");
    establish_connection(socket_fd, server_addr); // Connect to the server
    printf("Connected to naming server!\n");

    char server_reply[BUFFER_LENGTH] = {0}; // Initialize buffer
    char message[] = "Hello Server! This is Client.";

    transmit_message(socket_fd, message);
    printf("Transmitted message!\n");
    fetch_message(socket_fd, server_reply);

    printf("Received from server: %s\n", server_reply);

    printf("Received from server: %s\n", server_reply);

    close(socket_fd);
    return 0;
}
