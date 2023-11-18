#ifndef OPERATION_HANDLER_H
#define OPERATION_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "server_setup.h"

int connect_to_ss(char *ip, int port) // for privelleged functions
{
  int sock;
  struct sockaddr_in serv_addr, cli_addr;
  char server_ip[INET_ADDRSTRLEN]; // buffer for the IP address

  printf(CYN"Connecting to the SS at %s:%d\n"reset, ip, port);
  printf(CYN"Creating socket via port %d\n"reset, NM_SS_PORT_CONNECT);

  cli_addr.sin_family = AF_INET;
  cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  cli_addr.sin_port = htons(NM_SS_PORT_CONNECT);

  sock = createServerSocket();
  bindServerSocket(sock, &cli_addr);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0)
  {
    printf(RED"\nInvalid address/ Address not supported \n"reset);
    return -1;
  }

  connectToServer(sock, &serv_addr);

  return sock;
}

char *create(char *input)
{
  printf(GRN "Creating file %s\n" reset, input);
  char *folder_name = (char *)malloc(1000 * sizeof(char));
  char *file_name = (char *)malloc(1000 * sizeof(char));
  char *lastSlash;

  // Find the last occurrence of '\'
  lastSlash = strrchr(input, '/');

  if (lastSlash == NULL)
  {
    printf(RED "Invalid path\n" reset);
    return RED "Invalid path" reset;
  }

  int position = lastSlash - input;

  strncpy(folder_name, input, position);
  folder_name[position] = '\0'; // Null-terminate the string

  strcpy(file_name, lastSlash + 1);

  // Replace it with null character if found

  printf(GRN"Folder name is: %s\n"reset, folder_name);
  printf(GRN"File name is: %s\n"reset, file_name);

  ValueStruct *myStruct;
  if ((myStruct = find(accessible_paths_hashmap, folder_name)) == NULL)
  {
    printf(RED "Folder not found\n" reset);
    return RED "Folder not found" reset;
  }

  int sock = connect_to_ss(myStruct->ip, myStruct->nm_port);
  if (sock < 0)
  {
    printf(RED "Error in connecting to the storage server\n" reset);
    return RED "Error in connecting to the storage server" reset;
  }

  char buffer[4096] = {0};
  strcpy(buffer, "CREATE ");
  strcat(buffer, input);
  printf("> Sending message to SS : %s\n", buffer);
  sendMessage(sock, buffer);

  char *response = readMessage(sock);
  close(sock);

  if (strcmp(response, "1") == 0)
  {
    printf(GRN "File created successfully\n" reset);
    ValueStruct vs = {myStruct->ip, myStruct->nm_port, myStruct->nm_port, 0, 0};
    insert(accessible_paths_hashmap, input,
           vs); // insert the path and the value struct in the hashmap for the
                // new SS
    return GRN "File created successfully" reset;
  }
  else
  {
    printf(RED "Error in creating the file\n" reset);
    return RED "Error in creating the file" reset;
  }
}

char *delete_file(char *filename)
{

  printf(GRN "Deleting file %s\n" reset, filename);
  return GRN "Deleting the file" reset;
}

char *read_write_getinfo_file(char *filename)
{
  printf(GRN "Reading Writing or Getting Info of file %s\n" reset, filename);
  ValueStruct *myStruct;
  if ((myStruct = find(accessible_paths_hashmap, filename)) == NULL)
  {
    printf(RED "File not found\n" reset);
    return RED "File not found" reset;
  }

  // printf("IP is: %s\n", myStruct->ip);
  // printf("Client port is: %d\n", myStruct->client_port);
  // printf("NM port is: %d\n", myStruct->nm_port);
  // printf("Num readers is: %d\n", myStruct->num_readers);
  // printf("Is writing is: %d\n", myStruct->isWriting);
  // printf(GRN"IP is: %s\n"reset, myStruct->ip);

  // TODO : Add num_reader++, writing = 1;

  char response[1024];
  strcpy(response, "lookup response\nip:");
  strcat(response, myStruct->ip);
  strcat(response, "\nclient_port:");
  char client_port[20];
  sprintf(client_port, "%d", myStruct->client_port);
  strcat(response, client_port);

  char *response_final = (char *)malloc(10000 * sizeof(char));
  strcpy(response_final, response);
  return response_final;
}

// char *write_file(char *filename, char *data)
// {
//   printf(GRN "Writing '%s' to file %s\n" reset, data, filename);
//   return GRN "Writing to the file" reset;
// }

char *copy(char *filename1, char *filename2)
{
  printf(GRN "Copying file %s to %s\n" reset, filename1, filename2);
  return GRN "Copying the file" reset;
}

// char *get_info(char *filename)
// {
//   printf(GRN "Getting info of file %s\n" reset, filename);
//   return GRN "Getting info of the file" reset;
// }

char *LS()
{
  char *keys = get_all_keys(accessible_paths_hashmap);
  // printf("Keys are: %s\n", keys);
  char *response = (char *)malloc(1000);
  strcpy(response, GRN "Listing files in the directory" reset);
  strcat(response, "\n");
  strcat(response, keys);
  return response;
}

char *operation_handler(char **inputs, int num_inputs)
{
  if (strcmp(inputs[0], "LS") == 0)
  {
    // printf("Found LS!\n");
    return LS();
  }
  if (num_inputs == 2)
  {
    if (strcmp(inputs[0], "CREATE") == 0)
    {
      return create(inputs[1]);
    }
    else if (strcmp(inputs[0], "DELETE") == 0)
    {
      return delete_file(inputs[1]);
    }
    else if (strcmp(inputs[0], "READ") == 0)
    {
      return read_write_getinfo_file(inputs[1]);
    }
    else if (strcmp(inputs[0], "GETINFO") == 0)
    {
      return read_write_getinfo_file(inputs[1]);
    }
    else
    {
      printf(RED "Encountered invalid operation\n" reset);
      return RED "Invalid operation" reset;
    }
  }

  else if (strcmp(inputs[0], "WRITE") == 0)
  {
    return read_write_getinfo_file(inputs[1]);
  }

  else if (num_inputs == 3)
  {

    if (strcmp(inputs[0], "COPY") == 0)
    {
      return copy(inputs[1], inputs[2]);
    }
    else
    {
      printf(RED "Encountered invalid Operation\n" reset);
      return RED "Invalid operation" reset;
    }
  }

  else
  {
    printf(RED "Encountered invalid Operation\n" reset);
    return RED "Invalid operation" reset;
  }
}

#endif // OPERATION_HANDLER_H