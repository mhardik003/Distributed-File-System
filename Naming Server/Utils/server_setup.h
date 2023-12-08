#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#include "utils.h"

// Function Definitions
int createServerSocket();
void bindServerSocket(int server_fd, struct sockaddr_in *address);
void startListening(int server_fd);
int acceptConnection(int server_fd, struct sockaddr_in *address, char *ip_buffer);
void sendMessage(int socket, char *message);
char *readMessage(int sock);
void connectToServer(int sock, struct sockaddr_in *serv_addr);

int createServerSocket()
{
  /*
    Function to create a server socket

    Parameters:
    None

    Returns:
    int : the socket descriptor
  */

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1)
  {
    perror("Socket creation failed");
    // exit(EXIT_FAILURE);
  }

  int opt = 1;
  // Set SO_REUSEADDR to true
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    perror("setsockopt(SO_REUSEADDR) failed");
    close(sock);
    // exit(EXIT_FAILURE);
  }

  return sock;
}

void bindServerSocket(int server_fd, struct sockaddr_in *address)
{
  /*
    Function to bind the server socket to the provided address

    Parameters:
    int server_fd : the socket descriptor
    struct sockaddr_in *address : the address to bind to

    Returns:
    None
  */
  if (bind(server_fd, (struct sockaddr *)address, sizeof(*address)) < 0)
  {
    perror("bind failed");
    close(server_fd);
    // exit(EXIT_FAILURE);
  }
}

void startListening(int server_fd)
{
  /*
    Function to start listening for connections on the provided socket

    Parameters:
    int server_fd : the socket descriptor

    Returns:
    None
  */
  if (listen(server_fd, 5) < 0)
  {
    perror("listen");
    // exit(EXIT_FAILURE);
  }
}

int acceptConnection(int server_fd, struct sockaddr_in *address, char *ip_buffer)
{

  /*
    Function to accept a connection on the provided socket

    Parameters:
    int server_fd : the socket descriptor
    struct sockaddr_in *address : the address of the client
    char *ip_buffer : the buffer to store the IP address of the client

    Returns:
    int : the socket descriptor of the new connection
  */
  int addrlen = sizeof(*address);
  int new_socket = accept(server_fd, (struct sockaddr *)address, (socklen_t *)&addrlen);
  if (new_socket < 0)
  {
    perror("accept");
    // exit(EXIT_FAILURE);
  }

  // Copy the IP address to the provided buffer
  strncpy(ip_buffer, inet_ntoa(address->sin_addr), INET_ADDRSTRLEN);
  ip_buffer[INET_ADDRSTRLEN - 1] = '\0'; // Null-terminate the string

  return new_socket;
}

void sendMessage(int socket, char *message)
{
  /*
    Function to send a message to the provided socket

    Parameters:
    int socket : the socket descriptor
    char *message : the message to send

    Returns:
    None
  */
  if (send(socket, message, strlen(message), 0) < 0)
  {
    perror("Send failed");
    close(socket);
    // exit(EXIT_FAILURE);
  }
  // printf("Message sent!\n");
}

char *readMessage(int sock)
{
  /*
    Function to read a message from the provided socket

    Parameters:
    int sock : the socket descriptor

    Returns:
    char * : the message read from the socket
  */
  char *buffer = (char *)malloc(1024 * sizeof(char));
  if (buffer == NULL)
  {
    // Handle malloc failure
    close(sock);
    return NULL;
  }
  memset(buffer, 0, 1024);                       // Initialize buffer to zero
  ssize_t bytes_read = read(sock, buffer, 1023); // Read one less to leave room for null terminator

  if (bytes_read < 0)
  {
    // Handle read error
    free(buffer);
    close(sock);
    return "Error in reading the message";
  }

  buffer[bytes_read] = '\0'; // Manually null-terminate the string
  return buffer;
}

void connectToServer(int sock, struct sockaddr_in *serv_addr)
{
  /*
    Function to connect to the provided server

    Parameters:
    int sock : the socket descriptor
    struct sockaddr_in *serv_addr : the address of the server

    Returns:
    None
  */

  if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
  {
    perror(RED "Connection Failed" reset);
    close(sock);
    // exit(EXIT_FAILURE);
  }
}

#endif // SERVER_SETUP_H