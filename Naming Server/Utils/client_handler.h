#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H


#include "server_setup.h"
#include "utils.h"

char *parseInput(char *input);
char *readClientMessage(int sock);
void *handleClientConnection(void *arg);
void *listenForClients(void *arg);



char *parseInput(char *input) {
  // array to store tokens
  char **tokens = (char **)malloc(10 * sizeof(char *));

  for (int i = 0; i < 10; i++) {
    tokens[i] = (char *)malloc(100 * sizeof(char));
  }
  int i = 0;
  char *token = strtok(input, " ");
  while (token != NULL) {
    // printf("%s\n", token);
    // store token in tokens array
    strcpy(tokens[i++], token);
    token = strtok(NULL, " ");
  }
  return operation_handler(tokens, i);
}


char *readClientMessage(int sock) {
  // printf("Now reading the message\n");
  char buffer[BUFFER_RECV_SIZE] = {0};
  if (recv(sock, buffer, BUFFER_RECV_SIZE, 0) < 0) {
    return "Error in receiving the message";
  }
  printf("Message received by Client : %s", buffer);
  return parseInput(buffer);
}



void *handleClientConnection(void *arg) {
  connection_info *info = (connection_info *)arg;
  int sock = info->socket_desc;
  char ip_buffer[INET_ADDRSTRLEN]; // Buffer to store the IP address
  strncpy(ip_buffer, info->ip_address, INET_ADDRSTRLEN);

  free(info); // Free the allocated memory for the connection info

  while (1) {
    char *message = readClientMessage(sock);
    if (message == NULL || strcmp(message, "QUIT") == 0) {
      printf("Storage Server [%s] disconnected\n", ip_buffer);
      break;
    }
    sendMessage(sock, message);
    // free(message);
  }

  close(sock);
  return NULL;
}





void *listenForClients(void *arg) {
  int server_fd = *(int *)arg;
  while (1) {
    struct sockaddr_in address;
    char ip_buffer[INET_ADDRSTRLEN]; // Buffer to store the IP address

    int *new_sock = (int *)malloc(sizeof(int));
    *new_sock = acceptConnection(server_fd, &address, ip_buffer);
    printf("Connected to client %s\n", ip_buffer);

    if (*new_sock >= 0) {
      pthread_t thread_id;
      connection_info *info =
          (connection_info *)malloc(sizeof(connection_info));
      info->socket_desc = *new_sock;
      strncpy(info->ip_address, ip_buffer, INET_ADDRSTRLEN);
      pthread_create(&thread_id, NULL, handleClientConnection, (void *)info);
    }
  }
  return NULL;
}


#endif