#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include "server_setup.h"

char *parseInput(char *input);
char *readClientMessage(int sock);
void *handleClientConnection(void *arg);
void *listenForClients(void *arg);

#define WORD_SIZE_IN_INPUT 100
#define NUM_WORDS_IN_INPUT 100

char *parseInput(char *input)
{
  /*
    Function to parse the input from the client by breaking it into tokens by ' ' and
    then sending it forward to the operation_handler
  */

  // array to store tokens
  char **tokens = (char **)malloc(NUM_WORDS_IN_INPUT * sizeof(char *));
  if (!tokens)
  {
    printf("Memory allocation failed\n");
    return NULL;
    // exit(EXIT_FAILURE);
  }
  for (int i = 0; i < 10; i++)
  {
    tokens[i] = (char *)malloc(WORD_SIZE_IN_INPUT * sizeof(char));
    if (!tokens[i])
    {
      printf("Memory allocation failed\n");
      return NULL;
      // exit(EXIT_FAILURE);
    }
  }
  int i = 0;
  char *token = strtok(input, " ");
  while (token != NULL)
  {
    // printf("%s\n", token);
    strcpy(tokens[i++], token);
    token = strtok(NULL, " ");
  }
  return operation_handler(tokens, i);
}

char *readClientMessage(int sock)
{
  /*
    Function to read the message sent by the client
  */

  char buffer[BUFFER_RECV_SIZE] = {0};
  int read_bytes = recv(sock, buffer, BUFFER_RECV_SIZE, 0);
  if (read_bytes < 0)
  {
    perror("Error in receiving the message");
    return NULL; // Or handle the error as you see fit
  }
  else if (read_bytes == 0) // Client closed the connection
  {
    printf("Client has closed the connection\n");
    return NULL;
  }

  printf("Client > %s\n", buffer);
  return parseInput(buffer);
}

void *handleClientConnection(void *arg)
{
  /*
    Thread handler function for the client threads (called from listenForClients)
  */

  connection_info *info = (connection_info *)arg;
  int sock = info->socket_desc;
  char ip_buffer[INET_ADDRSTRLEN]; // Buffer to store the IP address
  strncpy(ip_buffer, info->ip_address, INET_ADDRSTRLEN);

  free(info); // Free the allocated memory for the connection info

  while (1)
  {
    char *message = readClientMessage(sock);
    if (message == NULL) // Check if client disconnected or an error occurred
    {
      break;
    }
    // readClient message takes the socket as input, reads whatever message the client has sent,
    // then it parses the input and performs the appropriate action and finally returns the appropriate
    // message which needs to be sent to the client
    sendMessage(sock, message);
    // free(message);
  }

  close(sock);
  return NULL;
}

void *listenForClients(void *arg)
{
  /*
    Function called from main which actively listens for client connections and spawns a new thread
    for each client and calls 'handleClientConnection' for each client
  */

  int server_fd = *(int *)arg;

  while (1)
  {
    struct sockaddr_in address;
    char ip_buffer[INET_ADDRSTRLEN]; // Buffer to store the IP address

    int *new_sock = (int *)malloc(sizeof(int));
    if (new_sock == NULL)
    {
      printf("Memory allocation failed\n");
      return NULL;
      // exit(EXIT_FAILURE);
    }
    *new_sock = acceptConnection(server_fd, &address, ip_buffer);
    printf(CYN "Connected to client %s\n" reset, ip_buffer);

    if (*new_sock >= 0)
    {
      pthread_t thread_id;
      connection_info *info = (connection_info *)malloc(sizeof(connection_info));
      
      if (info == NULL)
      {
        printf("Memory allocation failed\n");
        return NULL;
        // exit(EXIT_FAILURE);
      }

      info->socket_desc = *new_sock;
      strncpy(info->ip_address, ip_buffer, INET_ADDRSTRLEN);
      pthread_create(&thread_id, NULL, handleClientConnection, (void *)info);
    }
  }
  return NULL;
}

#endif