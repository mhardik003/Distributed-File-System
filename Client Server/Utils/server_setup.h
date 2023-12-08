#ifndef SERVER_SETUP_H
#define SERVER_SETUP_H

#define WHT "\e[0;37m"
#define BLK "\e[0;30m"
#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define MAG "\e[0;35m"
#define CYN "\e[0;36m"
#define WHT "\e[0;37m"
#define reset "\e[0m"

#define START_PORT 8000
#define MAX_PORT 65535

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define SERVER_PORT 8080
int CLIENT_NM_PORT;
int CLIENT_SS_PORT;

// Function Declarations
void print_DFS_features();
int findAvailablePort();

int createClientSocket();
void bindClientSocket(int sock, struct sockaddr_in *cli_addr);
void sendMessage(int socket, char *message);
char *readMessage(int sock);
void connectToServer(int sock, struct sockaddr_in *serv_addr);

void print_DFS_features()
{
    /*
        Function to print the features of the DFS
    */
    printf(WHT "_______________________________________________________________\n\n");
    printf("\t\t\tWelcome to the DFS!\n");
    printf("_______________________________________________________________\n\n");
    printf("You can perform the following 7 operations:\n");

    printf("1. Read a file\n");
    printf("Syntax: READ <path>\n\n");

    printf("2. Get information of a file/folder\n");
    printf("Syntax: GETINFO <path>\n\n");

    printf("3. Write to a file\n");
    printf("Syntax: WRITE <path> <text>\n\n");

    printf("4. Create a directory/file\n");
    printf("Syntax: CREATE <path>\n\n");

    printf("5. Delete a directory/file\n");
    printf("Syntax: DELETE <path>\n\n");

    printf("6. Display all files and folders in a folder\n");
    printf("Syntax: LS <path>\n\n");

    printf("7. Copy a directory/file to another filer or directory\n");
    printf("Syntax: COPY <source path> <destination path>\n\n" reset);
}

int findAvailablePort()
{
    /*
        Function to find an available port to bind to
        Parameters:
        None
        Returns:
        int : the available port
    */

    int test_sock;
    struct sockaddr_in test_addr;
    int port = START_PORT;
    while (port < MAX_PORT)
    {
        test_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (test_sock < 0)
        {
            perror("Socket creation error in findAvailablePort");
            // exit(EXIT_FAILURE);
        }

        memset(&test_addr, 0, sizeof(test_addr));
        test_addr.sin_family = AF_INET;
        test_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        test_addr.sin_port = htons(port);

        if (bind(test_sock, (struct sockaddr *)&test_addr, sizeof(test_addr)) < 0)
        {
            if (errno == EADDRINUSE)
            {
                // Port is already in use, try next port
                port++;
            }
            else
            {
                perror("Bind failed in findAvailablePort");
                // exit(EXIT_FAILURE);
            }
            close(test_sock);
        }
        else
        {
            // Found an available port
            close(test_sock);
            return port;
        }
    }
    return -1; // No available port found
}

int createClientSocket()
{
    /*
        Function to create a client socket
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

void bindClientSocket(int sock, struct sockaddr_in *cli_addr)
{
    /*
        Function to bind the client socket to the provided address
        Parameters:
        int sock : the socket descriptor
        struct sockaddr_in *cli_addr : the address to bind to
        Returns:
        None
    */

    if (bind(sock, (struct sockaddr *)cli_addr, sizeof(*cli_addr)) < 0)
    {
        perror("bind failed");
        close(sock);
        // exit(EXIT_FAILURE);
    }
}

void sendMessage(int socket, char *message)
{
    if (send(socket, message, strlen(message), 0) < 0)
    {
        perror("Send failed");
        close(socket);
        // exit(EXIT_FAILURE);
    }
    // printf("Message sent\n");
}

char *readMessage(int sock)
{
    // char buffer[1024] = {0};
    char *buffer = (char *)malloc(1024);

    if (!buffer)
    {
        printf("Memory allocation failed\n");
        return NULL;
        // exit(EXIT_FAILURE);
    }

    if (read(sock, buffer, 1024) < 0)
    {
        free(buffer);
        perror("Read failed");
        close(sock);
        return NULL;
    }
    return buffer;
}

void connectToServer(int sock, struct sockaddr_in *serv_addr)
{
    /*
        Function to connect to the server
        Parameters:
        int sock : the socket descriptor
        struct sockaddr_in *serv_addr : the address of the server
        Returns:
        None

    */
    if (connect(sock, (struct sockaddr *)serv_addr, sizeof(*serv_addr)) < 0)
    {
        perror("Connection Failed");
        close(sock);
        // exit(EXIT_FAILURE);
    }
}

#endif