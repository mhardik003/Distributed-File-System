#include "Utils/utils.h"

int main()
{

    struct sockaddr_in NM_client_address, NM_SS_address; // the addresses for connection with the Clients and the SS's

    // Address setup for both ports
    NM_client_address.sin_family = AF_INET;
    NM_client_address.sin_addr.s_addr = INADDR_ANY;
    NM_client_address.sin_port = htons(NM_Client_PORT);

    NM_SS_address.sin_family = AF_INET;
    NM_SS_address.sin_addr.s_addr = INADDR_ANY;
    NM_SS_address.sin_port = htons(NM_SS_PORT_LISTEN);

    // Hashmap creation
    // This hashmap stores the accessible_paths of the SS's as the key and the related SS info in a struct as a value to the key
    // init_hashmap(accessible_paths_hashmap);
    cache = (LRUCache *)malloc(sizeof(LRUCache));
    if (cache == NULL)
    {
        perror("Failed to allocate memory for LRUCache");
    }
    cacheInit(cache);

    // Socket creation and binding
    NM_SS_fd = createServerSocket();
    NM_client_fd = createServerSocket();

    bindServerSocket(NM_SS_fd, &NM_SS_address);
    bindServerSocket(NM_client_fd, &NM_client_address);

    // Start listening on both ports
    startListening(NM_client_fd);
    startListening(NM_SS_fd);

    pthread_t client_thread, ss_thread; // threads for listening and assigning individual threads to multiple clients and SS's

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
