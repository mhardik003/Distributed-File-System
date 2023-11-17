#include "Utils/utils.h"

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

    while(1) { // connection with naming server
        char message[1024];
        printf("Enter your message: ");
        fgets(message, 1024, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline character if present
        sendMessage(sock, message);
        char *nm_response = readMessage(sock); // Read confirmation message from server
        parseInput(nm_response, message); // Parses the client input and sends it to the operation handler
    }

    close(sock);
    return 0;
}
