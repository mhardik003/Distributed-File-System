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

    if(strncmp(buffer, "lookup response", 15) == 0) {
        char *token = strtok(buffer, "\n");
        token = strtok(NULL, "\n");
        char ip[100];
        sscanf(token, "ip:%s", ip);
        token = strtok(NULL, "\n");
        int port;
        sscanf(token, "client_port:%d", &port);
        // printf("IP is: %s\n", ip);
        // printf("Port is: %d\n", port);
        printf("Sending request to the storage server at the IP address %s and port %d\n", ip, port);
        
        // create a new socket for communication with the storage server
        int sock;
        struct sockaddr_in serv_addr, cli_addr;
        char server_ip[INET_ADDRSTRLEN]; // Buffer for the IP address

        cli_addr.sin_family = AF_INET;
        cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        cli_addr.sin_port = htons(CLIENT_SS_PORT);

        sock = createClientSocket();
        bindClientSocket(sock, &cli_addr);

        memset(&serv_addr, 0, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(port);

        if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
            printf("\nInvalid address/ Address not supported \n");
            return;
        }

        connectToServer(sock, &serv_addr);
        sendMessage(sock, "Send me the file\n");
        readMessage(sock);
    }

    else {
        printf("%s\n", buffer);
    }
}

void print_DFS_features() {
    printf("_______________________________________________________________\n");
    printf("\t\t\tWelcome to the DFS!\n");
    printf("_______________________________________________________________\n");
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

int main() {
    CLIENT_NM_PORT = findAvailablePort();
    CLIENT_SS_PORT = findAvailablePort();
    if (CLIENT_NM_PORT == -1 || CLIENT_SS_PORT == -1) {
        printf("No available port found. Exiting.\n");
        return -1;
    }

    printf("\nClient Port for communication with naming server: %d\n", CLIENT_NM_PORT);
    printf("Client Port for communication with storage server: %d\n\n", CLIENT_SS_PORT);
    int sock;
    struct sockaddr_in serv_addr, cli_addr;
    char server_ip[INET_ADDRSTRLEN]; // Buffer for the IP address

    cli_addr.sin_family = AF_INET;
    cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    cli_addr.sin_port = htons(CLIENT_NM_PORT);

    sock = createClientSocket();
    bindClientSocket(sock, &cli_addr);

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
    print_DFS_features();

    while(1) {
        char message[1024];
        printf("Enter your message: ");
        fgets(message, 1024, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline character if present
        sendMessage(sock, message);
        readMessage(sock); // Read confirmation message from server
    }

    close(sock);
    return 0;
}
