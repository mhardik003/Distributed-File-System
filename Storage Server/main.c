#include "Utils/client_handler.h"


// void *listenFromNameServer(int *NM_SS_fd)
// {
//     int ss_fd = *NM_SS_fd;
//     int new_socket;
//     struct sockaddr_in ss_address;
//     int addrlen = sizeof(ss_address);

//     while (1)
//     {
//         if ((new_socket = accept(ss_fd, (struct sockaddr *)&ss_address, (socklen_t *)&addrlen)) < 0)
//         {
//             perror("accept");
//             exit(EXIT_FAILURE);
//         }
//         printf("Storage Server connected!\n");
//         char buffer[1024] = {0};
//         read(new_socket, buffer, 1024);
//         printf("%s\n", buffer);
//         send(new_socket, "Hello from server", strlen("Hello from server"), 0);
//         printf("Hello message sent\n");
//     }
// }

int main()
{
    // SS_NM_PORT, SS_Client_PORT;
    SS_NM_PORT = findAvailablePort();
    SS_Client_PORT = findAvailablePort();

    if (SS_NM_PORT == -1 || SS_Client_PORT == -1)
    {
        printf("No available port found. Exiting.\n");
        return -1;
    }

    printf("\nStorage Server Port for communication with naming server: %d\n", SS_NM_PORT);
    printf("Storage Server Port for communication with client: %d\n\n", SS_Client_PORT);

    int SS_client_fd, SS_NM_fd;
    struct sockaddr_in SS_client_address, SS_NM_address;

    char server_ip[INET_ADDRSTRLEN]; // Buffer for the IP address

    SS_client_address.sin_family = AF_INET;
    SS_client_address.sin_addr.s_addr = INADDR_ANY;
    SS_client_address.sin_port = htons(SS_Client_PORT);

    // SS_NM_address.sin_family = AF_INET;
    // SS_NM_address.sin_addr.s_addr = INADDR_ANY;
    // SS_NM_address.sin_port = htons(SS_NM_PORT);

    SS_NM_address.sin_family = AF_INET;
    SS_NM_address.sin_addr.s_addr = htonl(INADDR_ANY);
    SS_NM_address.sin_port = htons(SS_NM_PORT);

    SS_NM_fd = createServerSocket();
    SS_client_fd = createServerSocket();

    bindServerSocket(SS_NM_fd, &SS_NM_address);
    bindServerSocket(SS_client_fd, &SS_client_address);

    sendInfoToNM(server_ip, SS_NM_fd, SS_NM_address);

    // startListening(SS_client_fd);
    // printf("billu bageele\n");
    // startListening(SS_NM_fd);

    pthread_t client_thread, nm_thread;

    startListening(SS_client_fd);
    pthread_create(&client_thread, NULL, listenForClients, (void *)&SS_client_fd);
    // pthread_create(&nm_thread, NULL, listenFromNameServer, (void *)&SS_NM_fd);

    pthread_join(client_thread, NULL);

    // while(1) {
    // char message[1024];
    // printf(CYN"Enter Message: "reset);
    // fgets(message, 1024, stdin);
    // message[strcspn(message, "\n")] = 0; // Remove newline character if present
    // sendMessage(sock, message);
    // readMessage(sock); // Read confirmation message from server
    // }

    close(SS_client_fd);
    close(SS_NM_fd);
    return 0;
}
