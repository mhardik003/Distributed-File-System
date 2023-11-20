#ifndef SS_HANDLER_H
#define SS_HANDLER_H

#include <string.h>
// #include "operation_handler.h"
#include "utils.h"
int num_storage_servers = 0;

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
    sem_t write_semaphore, read_semaphore;
    sem_init(&write_semaphore, 0, 1);
    sem_init(&read_semaphore, 0, 1);
    ValueStruct vs = {ipAddrCharArray, ss_nm_port, ss_client_port, 0, 0, write_semaphore, read_semaphore};
    putInCache(cache, token, vs); // insert the path and the value struct in the hashmap for the new SS

    // ValueStruct *myStruct = find(accessible_paths_hashmap, token);
    // printf("IP is: %s\n", myStruct->ip);
    // printf("Client port is: %d\n", myStruct->client_port);
    // printf("NM port is: %d\n", myStruct->nm_port);
    // printf("Num readers is: %d\n", myStruct->num_readers);
    // printf("Is writing is: %d\n", myStruct->isWriting);
    token = strtok(NULL, "\n");
  }

  // Insert the SS info in the SS list
  insertNode(&head_list, ip_address, ss_nm_port, ss_client_port);
  printf("List of all connected servers : \n");
  displayList(head_list);

  // Increment the number of servers
  num_storage_servers++;
  printf(CYN "NM > Number of storage servers connected: %d\n" reset, num_storage_servers);

  return 1;
}

char *process_SS_msg(int sock, char *ip_address)
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
  int bytes_read = recv(sock, buffer, BUFFER_RECV_SIZE, 0);
  if (bytes_read < 0)
  {
    return RED "NM > Error in receiving the message" reset;
  }
  else if (bytes_read == 0)
  {
    printf(RED "NM > Storage server %s has closed the connection\n" reset, ip_address);
    num_storage_servers--;
    return NULL;
  }
  if (strncmp(buffer, "ssinit", 6) == 0)
  {
    printf(GRN "%s" reset, buffer);
    parse_ssinit(buffer, ip_address);

    return "ssinit done";
  }
  else if (strncmp(buffer, "ACK", 3) == 0)
  {
    char *ackToken = strtok(buffer, "\n");
    ackToken = strtok(NULL, "\n");
    char *ackPath = strtok(NULL, "\n");
    printf(GRN "SS > Acknowledgement for '%s' operation\n" reset, ackToken);
    if (strcmp(ackToken, "READ") == 0 || strcmp(ackToken, "GETINFO") == 0)
    {
      ValueStruct *myStruct = getFromCache(cache, ackPath);
      sem_wait(&(myStruct->read_semaphore));
      myStruct->num_readers--;
      sem_post(&(myStruct->read_semaphore));
      printf(MAG "One reader completed reading. Number of readers for %s is %d\n" reset, ackPath, myStruct->num_readers);
    }
    else if (strcmp(ackToken, "WRITE") == 0)
    {
      ValueStruct *myStruct = getFromCache(cache, ackPath);
      sem_wait(&(myStruct->write_semaphore));
      myStruct->isWriting = 0;
      sem_post(&(myStruct->write_semaphore));
      printf(MAG "Writer completed writing. isWriting for %s is %d\n" reset, ackPath, myStruct->isWriting);
    }
  }
  printf("SS > %s\n", buffer);
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
  char *message = process_SS_msg(sock, ip_buffer);
  sendMessage(sock, message);

  // sendMessage(sock, "\nCREATE bkps/");
  // }
  close(sock);

  // TODO
  if (num_storage_servers == 3)
  {
    printf("Starting to create the backups now. \n");
    backup_init(); // initialize and complete the backups of the 3 servers
  }
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
    printf(CYN "NM > Connected to storage server with IP %s\n" reset, ip_buffer);

    if (*new_sock >= 0) // if the connection is successful
    {
      pthread_t thread_id;
      connection_info *info =
          (connection_info *)malloc(sizeof(connection_info));
      info->socket_desc = *new_sock;

      strncpy(info->ip_address, ip_buffer, INET_ADDRSTRLEN);
      pthread_create(&thread_id, NULL, handleStorageServerConnection, (void *)info);
    }
  }
  return NULL;
}

#endif // SS_HANDLER_H