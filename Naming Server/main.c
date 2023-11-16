#include "Utils/utils.h"


int main() {
    int NM_client_fd, NM_SS_fd;
    struct sockaddr_in NM_client_address, NM_SS_address;

    // Address setup for both ports
    NM_client_address.sin_family = AF_INET;
    NM_client_address.sin_addr.s_addr = INADDR_ANY;
    NM_client_address.sin_port = htons(NM_Client_PORT);

    NM_SS_address.sin_family = AF_INET;
    NM_SS_address.sin_addr.s_addr = INADDR_ANY;
    NM_SS_address.sin_port = htons(NM_SS_PORT);

    // Hashmap creation
    init_hashmap(accessible_paths_hashmap);

    // Socket creation and binding
    NM_client_fd = createServerSocket();
    NM_SS_fd = createServerSocket();

    bindServerSocket(NM_client_fd, &NM_client_address);
    bindServerSocket(NM_SS_fd, &NM_SS_address);

    // Start listening on both ports
    startListening(NM_client_fd);
    startListening(NM_SS_fd);

    pthread_t client_thread, ss_thread;

    pthread_create(&client_thread, NULL, listenForClients, (void *)&NM_client_fd);
    pthread_create(&ss_thread, NULL, listenForStorageServers, (void *)&NM_SS_fd);

    // Join threads (optional, depending on your shutdown strategy)
    pthread_join(client_thread, NULL);
    pthread_join(ss_thread, NULL);

    // Cleanup
    close(NM_client_fd);
    close(NM_SS_fd);
    return 0;
}
