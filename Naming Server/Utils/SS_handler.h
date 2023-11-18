#ifndef SS_HANDLER_H
#define SS_HANDLER_H

#include <string.h>

int parse_ssinit(char *init_message, char *ip_address)
{
  /*
  Parses the init message sent by the storage server
  and stores the information in the hashmap

    Parameters:
    char *init_message : init message sent by the storage server
    char *ip_address : IP address of the storage server

    Returns:
    int : 1 if the parsing is successful, 0 otherwise

  The init message consists of
  * List of paths accessible by the storage server
  * IP address of the storage server
  * Port number of the Storage Server for communication with the Naming Server
  * Port number of the Storage Server for communication with the client\
  */
  char *token;
  int ss_nm_port, ss_client_port;

  token = strtok(init_message, "\n");
  if (token == NULL || strcmp(token, "ssinit") != 0)
  {
    return 0;
  }

  token = strtok(NULL, "\n");
  if (token == NULL ||
      sscanf(token, "%d %d", &ss_nm_port, &ss_client_port) != 2)
  {
    return 0;
  }

  token = strtok(NULL, "\n");
  while (token != NULL)
  {
    char ipAddrCharArray[100];
    strcpy(ipAddrCharArray, ip_address);
    ValueStruct vs = {ipAddrCharArray, ss_nm_port, ss_client_port, 0, 0};
    insert(accessible_paths_hashmap, token,
           vs); // insert the path and the value struct in the hashmap for the
                // new SS

    // ValueStruct *myStruct = find(accessible_paths_hashmap, token);
    // printf("IP is: %s\n", myStruct->ip);
    // printf("Client port is: %d\n", myStruct->client_port);
    // printf("NM port is: %d\n", myStruct->nm_port);
    // printf("Num readers is: %d\n", myStruct->num_readers);
    // printf("Is writing is: %d\n", myStruct->isWriting);
    token = strtok(NULL, "\n");
  }

  return 1;
}

char *readStorageServerMessage(int sock, char *ip_address)
{
  /*
    Function to read the message sent by the storage server

    Parameters:
    int sock : socket descriptor of the storage server
    char *ip_address : IP address of the storage server

    Returns:
    char * : message sent by the storage server
  */

  // printf("Now reading the message\n");
  char buffer[BUFFER_RECV_SIZE] = {0};
  if (recv(sock, buffer, BUFFER_RECV_SIZE, 0) < 0)
  {
    return "Error in receiving the message";
  }
  printf("Message received by SS : %s", buffer);
  if (strncmp(buffer, "ssinit", 6) == 0)
  {
    parse_ssinit(buffer, ip_address);
    return "ssinit done";
  }
  return "Hey SS, how are you? -NM";
}

void *handleStorageServerConnection(void *arg)
{
  /*
    Function to handle the connection with the storage server

    Parameters:
    void *arg : pointer to the connection_info struct

    Returns:
    void * : NULL
  */

  connection_info *info = (connection_info *)arg;
  int sock = info->socket_desc;
  char ip_buffer[INET_ADDRSTRLEN];
  strncpy(ip_buffer, info->ip_address, INET_ADDRSTRLEN);

  free(info); // Free the allocated memory for the connection info

  // while (1) {
  char *message = readStorageServerMessage(sock, ip_buffer);
  if (message == NULL || strcmp(message, "QUIT") == 0)
  {
    printf("Storage Server [%s] disconnected\n", ip_buffer);
    close(sock);
    return NULL;
    // break;
  }
  sendMessage(sock, message);
  // }

  close(sock);
  return NULL;
}

void *listenForStorageServers(void *arg)
{
  /*
    Function to listen for connections from the storage servers

    Parameters:
    void *arg : pointer to the server_fd

    Returns:
    void * : NULL
  */
 
  int server_fd = *(int *)arg;
  while (1)
  {
    struct sockaddr_in address;
    char ip_buffer[INET_ADDRSTRLEN]; // Buffer to store the IP address

    int *new_sock = (int *)malloc(sizeof(int));
    *new_sock = acceptConnection(server_fd, &address, ip_buffer);
    printf("Connected to storage server %s\n", ip_buffer);

    if (*new_sock >= 0)
    {
      pthread_t thread_id;
      connection_info *info =
          (connection_info *)malloc(sizeof(connection_info));
      info->socket_desc = *new_sock;
      strncpy(info->ip_address, ip_buffer, INET_ADDRSTRLEN);

      pthread_create(&thread_id, NULL, handleStorageServerConnection,
                     (void *)info);
    }
  }
  return NULL;
}

#endif // SS_HANDLER_H