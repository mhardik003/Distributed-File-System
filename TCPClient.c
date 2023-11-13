#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define SERVER_PORT 8080
#define BUFFER_LENGTH 1024

// Function to display error message and exit the program
void display_error(const char *error_msg) {
    printf("\033[1;31mERROR: %s\n\033[0m", error_msg);
    exit(EXIT_FAILURE);
}

// Function to retrieve the IP address of the host machine
char* retrieve_ip_address() {
    char host_buffer[256];
    struct hostent *host_info;
    
    // Get host name
    if (gethostname(host_buffer, sizeof(host_buffer)) != 0) 
        display_error("Unable to obtain IP address");

    // Get host entry info
    host_info = gethostbyname(host_buffer);
    if (host_info == NULL) 
        display_error("Unable to obtain IP address");

    return inet_ntoa(*((struct in_addr*)host_info->h_addr_list[0]));
}

// Function to create a socket and return its file descriptor
int initialize_socket() {
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1) 
        display_error("Socket creation failed");
    return socket_fd;
}

// Function to connect to the server using the provided socket file descriptor and server address
void establish_connection(int socket_fd, struct sockaddr_in server_addr) {
    if (connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) 
        display_error("Connection to server failed");
}

// Function to send a message to the server
void transmit_message(int socket_fd, const char* message) {
    if (send(socket_fd, message, strlen(message), 0) == -1) 
        display_error("Message transmission to server failed");
}

// Function to receive a message from the server
void fetch_message(int socket_fd, char* server_reply) {
    if (recv(socket_fd, server_reply, BUFFER_LENGTH, 0) < 0) 
        display_error("No data received from server");
}

int main() {
    char* ip_address = retrieve_ip_address();
    int socket_fd = initialize_socket();

    struct sockaddr_in server_addr;
    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(ip_address);
    server_addr.sin_port = htons(SERVER_PORT);

    establish_connection(socket_fd, server_addr);

    char server_reply[BUFFER_LENGTH];
    // char* message = "Hello Server! This is Client sending you a message.\0";
    char message[256] = "\0";
    strcpy(message, "Hello Server! This is Client sending you a message.\0");

    transmit_message(socket_fd, message);
    fetch_message(socket_fd, server_reply);

    printf("Received from server: %s\n", server_reply);

    close(socket_fd);
    return 0;
}
