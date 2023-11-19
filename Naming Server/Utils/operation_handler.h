#ifndef OPERATION_HANDLER_H
#define OPERATION_HANDLER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"
#include "server_setup.h"

char *operation_handler(char **inputs, int num_inputs);
int connect_to_ss(char *SS_ip, int SS_port);
ValueStruct *check_existence(char *input_check);
char *create(char *input);
char *delete_file(char *filename);
char *read_write_getinfo_file(char *filename, char *operation);
char *create_file_in_destination_server(ValueStruct *SS2, char *destination_path);
char *create_folder_in_destination_server(ValueStruct *SS2, char *destination_path);
char *read_and_save_contents_from_source_server(ValueStruct *SS1, char *source);
char *write_contents_to_destination_server(ValueStruct *SS2, char *destination_path, char *source);
char *copy_file(char *source, char *destination);
char *copy_directory(char *source, char *destination);
char *copy(char *filename1, char *filename2);
char *LS();

char *operation_handler(char **inputs, int num_inputs)
{
  /*
    Main function to handle the operations which calls the relevant function for the operations
  */
  if (strcmp(inputs[0], "LS") == 0)
  {
    return LS();
  }
  if (num_inputs == 2) // for all the functions which take 2 inputs
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
      return read_write_getinfo_file(inputs[1], "GETINFO");
    }
    else if (strcmp(inputs[0], "GETINFO") == 0)
    {
      return read_write_getinfo_file(inputs[1], "READ");
    }
    else
    {
      printf(RED "Encountered invalid operation\n" reset);
      return RED "> NM : Invalid operation" reset;
    }
  }

  else if (strcmp(inputs[0], "WRITE") == 0) // for write operation
  {
    return read_write_getinfo_file(inputs[1], "WRITE");
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
      return RED "> NM : Invalid operation" reset;
    }
  }

  else
  {
    printf(RED "Encountered invalid Operation\n" reset);
    return RED "> NM : Invalid operation" reset;
  }
}

int connect_to_ss(char *SS_ip, int SS_port)
{
  /*
    Function to create a connection for privileged functions between NM and SS
  */

  int sock;
  struct sockaddr_in serv_addr, cli_addr;
  char server_ip[INET_ADDRSTRLEN]; // buffer for the IP address

  printf(CYN "Connecting to the SS at %s:%d\n" reset, SS_ip, SS_port);
  printf(CYN "Creating socket via port %d\n" reset, NM_SS_PORT_CONNECT);

  cli_addr.sin_family = AF_INET;
  cli_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  cli_addr.sin_port = htons(NM_SS_PORT_CONNECT);

  sock = createServerSocket();
  bindServerSocket(sock, &cli_addr);

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(SS_port);

  if (inet_pton(AF_INET, SS_ip, &serv_addr.sin_addr) <= 0)
  {
    printf(RED "\nInvalid address/ Address not supported \n" reset);
    return -1;
  }

  connectToServer(sock, &serv_addr); // connecting to the SS

  return sock; // return this socket which has a connection between the Naming Server and the required Storage Server
}

ValueStruct *check_existence(char *input_check)
{
  /*
    Check if the accessible path is present in the Hashmap
    Used for the create function to check if the path is valid

    Returns the related info to the accessible path if found else NULL
  */

  char *lastSlash;
  char *folder_name = (char *)malloc(1000 * sizeof(char));
  char *file_name = (char *)malloc(1000 * sizeof(char));

  // Find the last occurrence of '/'
  lastSlash = strrchr(input_check, '/');

  if (lastSlash == NULL)
  {
    printf(RED "Invalid path\n" reset);
    return NULL;
  }

  // Check if the input ends in a '/'
  if (input_check[strlen(input_check) - 1] == '/')
  {
    // Remove the last '/'
    input_check[strlen(input_check) - 1] = '\0';
  }

  lastSlash = strrchr(input_check, '/');

  if (lastSlash == NULL)
  {
    printf(RED "Trying to make a new root folder\n" reset);
    return NULL;
  }

  int position = lastSlash - input_check;

  strncpy(folder_name, input_check, position);
  folder_name[position] = '/';
  folder_name[position + 1] = '\0'; // Null-terminate the string

  strcpy(file_name, lastSlash + 1);

  printf(GRN "Folder name is: %s\n" reset, folder_name);
  printf(GRN "File name is: %s\n" reset, file_name);

  ValueStruct *myStruct;
  if ((myStruct = find(accessible_paths_hashmap, folder_name)) == NULL)
  {
    printf(RED "Folder not found\n" reset);
    return NULL;
  }

  return myStruct;
}

char *create(char *input)
{

  printf(GRN "Creating file %s\n" reset, input);
  char *input_check;

  // copy the input to a new string
  input_check = (char *)malloc(1000 * sizeof(char));
  strcpy(input_check, input);

  ValueStruct *myStruct;
  myStruct = check_existence(input_check);

  if (check_existence(input_check) == NULL)
  {
    return RED "> NM : Invalid path or trying to make a file/folder in root\n" reset;
  }

  // Establish a connection with the SS after checking the existence of the folder in the SS
  int sock = connect_to_ss(myStruct->ip, myStruct->nm_port);
  if (sock < 0)
  {
    printf(RED "Error in connecting to the storage server\n" reset);
    return RED "> NM : Error in connecting to the storage server" reset;
  }

  char message_to_SS[4096] = {0};
  strcpy(message_to_SS, "CREATE ");
  strcat(message_to_SS, input);
  printf("SS < %s\n", message_to_SS);
  sendMessage(sock, message_to_SS); //  send the message to the SS to create the file/folder

  char *response = readMessage(sock); // receive! the response message from the SS
  close(sock);

  if (strcmp(response, "CREATED") == 0) // if creation was a success
  {
    printf(GRN "> SS : File/Folder created successfully\n" reset);
    ValueStruct vs = {myStruct->ip, myStruct->nm_port, myStruct->nm_port, 0, 0};
    insert(accessible_paths_hashmap, input,
           vs); // insert the path and the value struct in the hashmap for the
                // new SS
    return GRN "> NM : File/Folder created successfully" reset;
  }
  else
  {
    printf(RED "Error in creating the file\n" reset);
    return RED "> NM : Error in creating the file" reset;
  }
}

char *delete_file(char *filename)
{
  /*
    Function to delete file via NM
  */

  printf(GRN "Deleting file %s\n" reset, filename);

  ValueStruct *myStruct;
  if ((myStruct = find(accessible_paths_hashmap, filename)) == NULL)
  {
    printf(RED "File not found\n" reset);
    return RED "> NM : File not found" reset;
  }

  int sock = connect_to_ss(myStruct->ip, myStruct->nm_port); // establish a connection with the SS

  if (sock < 0)
  {
    printf(RED "Error in connecting to the storage server\n" reset);
    return RED "> NM : Error in connecting to the storage server" reset;
  }

  char message_to_SS[4096] = {0};
  strcpy(message_to_SS, "DELETE ");
  strcat(message_to_SS, filename);
  printf("SS < %s\n", message_to_SS);
  sendMessage(sock, message_to_SS);

  char *response = readMessage(sock); // RECEIVE
  close(sock);

  if (strcmp(response, "DELETED FILE") == 0) // if the file/folde was successfully deleted
  {
    printf(GRN "SS > File deleted successfully\n" reset);
    remove_key(accessible_paths_hashmap, filename);
    return GRN "> NM : File deleted successfully" reset;
  }
  else if (strcmp(response, "DELETED FOLDER") == 0)
  {
    printf(GRN "SS > Directory deleted successfully\n" reset);
    remove_folder(accessible_paths_hashmap, filename);
    return GRN "> NM : Directory deleted successfully" reset;
  }
  else
  {
    printf(RED "Error in deleting the file\n" reset);
    return RED "> NM : Error in deleting the file" reset;
  }

  return GRN "> NM : Deleting the file" reset;
}

char *read_write_getinfo_file(char *filename, char *operation)
{
  /*
    For Client-SS related functions where this function (read, write, getinfo)
    returns the IP and Port of the SS to the Client
  */

  printf(GRN "Reading Writing or Getting Info of file %s\n" reset, filename);

  ValueStruct *myStruct;
  if ((myStruct = find(accessible_paths_hashmap, filename)) == NULL)
  {
    printf(RED "File not found\n" reset);
    return RED "> NM : File not found" reset;
  }

  if (strcmp(operation, "READ") || strcmp(operation, "GETINFO"))
  {
    printf(MAG "Incrementing num readers to %d for file %s\n" reset, myStruct->num_readers + 1, filename);
    myStruct->num_readers++; // increase the number of readers for that file
  }

  else if (strcmp(operation, "WRITE"))
  {
    if (myStruct->isWriting == 1)
    {
      return RED "> NM : More than one client cannot write!" reset;
    }

    printf(MAG "Setting isWriting to 1 for file %s\n" reset, filename);
    myStruct->isWriting = 1; // make the boolean flag to 1, so that now no one else can write to this file until this client is done
  }

  char* response = (char *)malloc(1000 * sizeof(char));
  strcpy(response, "lookup response\nip:");
  strcat(response, myStruct->ip);
  strcat(response, "\nclient_port:");
  char client_port[20];
  sprintf(client_port, "%d", myStruct->client_port);
  strcat(response, client_port);

  return response;
}

char *create_file_in_destination_server(ValueStruct *SS2, char *destination_path)
{
  // connect to SS2 to create the file there

  char *message_to_SS = (char *)malloc(1024 * sizeof(char));
  int sock2 = connect_to_ss(SS2->ip, SS2->nm_port);
  // create the file in destination server
  strcpy(message_to_SS, "CREATE ");
  strcat(message_to_SS, destination_path);

  printf("SS < %s\n", message_to_SS);
  sendMessage(sock2, message_to_SS);

  char *response = readMessage(sock2);
  if (strcmp(response, "CREATED") == 0)
  {
    printf(GRN "SS > File created successfully in the destination SS\n" reset);
  }
  else
  {
    printf(RED "Error in creating the file\n" reset);
    return RED "> NM : Error in creating the file" reset;
  }

  ValueStruct vs = {SS2->ip, SS2->nm_port, SS2->client_port, 0, 0};
  insert(accessible_paths_hashmap, destination_path, vs); // insert the path and the value struct in the hashmap for the destination SS

  printf(GRN "SS > %s\n" reset, response);
  printf("Closing the socket\n");
  close(sock2);
}

char *create_folder_in_destination_server(ValueStruct *SS2, char *destination_path)
{
  // connect to SS2 to create the folder there
  char *foldername = (char *)malloc(1024 * sizeof(char));
  char *message_to_SS = (char *)malloc(1024 * sizeof(char));

  int sock2 = connect_to_ss(SS2->ip, SS2->nm_port);

  // create the folder in destination server
  strcpy(message_to_SS, "CREATEFOLDER ");
  strcat(message_to_SS, destination_path);

  printf("SS < %s\n", message_to_SS);
  sendMessage(sock2, message_to_SS);

  char *response = readMessage(sock2);

  if (strcmp(response, "CREATED") == 0)
  {
    printf(GRN "SS > Folder created successfully in the destination SS\n" reset);
  }
  else
  {
    printf(RED "Error in creating the file\n" reset);
    return RED "> NM : Error in creating the file" reset;
  }

  // Append the destination+foldername to the hashmap with the ip and port of the destination SS
  ValueStruct vs = {SS2->ip, SS2->nm_port, SS2->client_port, 0, 0};
  insert(accessible_paths_hashmap, destination_path, vs); // insert the path and the value struct in the hashmap for the destination SS

  printf(GRN "SS > %s\n" reset, response);
  // printf("Closing the socket\n");
  close(sock2);
}

char *read_and_save_contents_from_source_server(ValueStruct *SS1, char *source)
{
  /*
   Function to read the contents of a file from a source server and
   then store it in a temporary file on the Naming Server
  */

  // SS1 is the source server
  char *filename = (char *)malloc(1024 * sizeof(char));
  char *message_to_SS = (char *)malloc(1024);
  char *tempSourcePath = (char *)malloc(1024 * sizeof(char));
  int sock1 = connect_to_ss(SS1->ip, SS1->nm_port);

  strcpy(tempSourcePath, source);

  filename = strrchr(tempSourcePath, '/');
  filename++;

  strcat(filename, "_temp");
  FILE *fp = fopen(filename, "w"); // create the temporary file where the contents of the file to be copied will be stored

  if (fp == NULL)
  {
    printf(RED "Error in creating the temporary file\n" reset);
    return RED "> NM : Error in creating the temporary file" reset;
  }

  printf("> NM :  Created the temporary file in the NM!\n");

  strcpy(message_to_SS, "NMREAD ");
  // printf("The source path is  : %s", source);
  strcat(message_to_SS, source);
  printf("SS < %s\n", message_to_SS);
  sendMessage(sock1, message_to_SS);

  // read the file from SS1 in chunks and store it in a temporary_file
  printf(GRN "SS > ");
  while (1)
  {
    char *SS_response = readMessage(sock1);
    // puts(SS_response);
    int resp_len = strlen(SS_response);

    if (strncmp(SS_response + resp_len - 3, "END", 3) == 0)
    {
      SS_response[resp_len - 3] = '\0';
      fputs(SS_response, fp);
      break;
    }
    // append the contents to the temporary file
    fputs(SS_response, fp);
  }
  printf("\n");
  close(sock1);
  fclose(fp);

  return "NM > Created file in destination server";
}

char *write_contents_to_destination_server(ValueStruct *SS2, char *destination_path, char *source)
{
  /*
    Function to write the contents of the temporary file to the destination file path
    and then delete the temporary file
  */

  char *filename = (char *)malloc(1024 * sizeof(char));
  char *message_to_ss = (char *)malloc(1024 * sizeof(char));
  char *tempSourcePath = (char *)malloc(1024);
  char *file_contents = (char *)malloc(10000 * sizeof(char));

  // Create the message to be sent to the SS
  strcpy(message_to_ss, "NMWRITE ");
  strcat(message_to_ss, destination_path);
  strcat(message_to_ss, " ");

  strcpy(tempSourcePath, source);

  filename = strrchr(tempSourcePath, '/');
  filename++;
  strcat(filename, "_temp");

  // open the temporary file
  FILE *fp = fopen(filename, "r");
  if (fp == NULL)
  {
    printf(RED "Error in opening the temporary file\n" reset);
    return RED "> NM : Error in opening the temporary file" reset;
  }

  // read and store the filecontents in a string
  char *line = (char *)malloc(1024);
  while (fgets(line, 1024, fp) != NULL)
  {
    // Check for message_to_ss overflow
    if (strlen(message_to_ss) + strlen(line) >= 10000)
    {
      printf("message_to_ss overflow - file is too large\n");
      break;
    }
    strcat(message_to_ss, line); // Append the line to the buffer
  }
  fclose(fp);

  int sock1 = connect_to_ss(SS2->ip, SS2->nm_port);
  sendMessage(sock1, message_to_ss); // send the message to the SS to write the contents to the destination file path
  char *response = readMessage(sock1);

  // delete the temporary file
  remove(filename);

  if (strcmp(response, "WRITTEN") == 0)
  {
    printf(GRN "SS > File copied successfully\n" reset);
    return GRN "> NM : File copied successfully" reset;
  }

  close(sock1);
  return RED "Error in copying" reset;
}

char *copy_file(char *source, char *destination)
{
  /*
     Function called if the client wants to copy a file
  */

  ValueStruct *SS1;
  ValueStruct *SS2;

  char *destination_path = (char *)malloc(1000 * sizeof(char));
  char *fileName = (char *)malloc(1024 * sizeof(char));
  char *tempSourcePath = (char *)malloc(1024 * sizeof(char));

  strcpy(destination_path, destination);
  strcpy(tempSourcePath, source);

  fileName = strrchr(tempSourcePath, '/');
  fileName++;

  strcat(destination_path, fileName);

  SS1 = find(accessible_paths_hashmap, source);
  SS2 = find(accessible_paths_hashmap, destination);

  // Create empty file in the destination server
  create_file_in_destination_server(SS2, destination_path);

  // connect to SS1 to read the contents of the file to be copied
  read_and_save_contents_from_source_server(SS1, source);

  // connect to SS2 to write the contents from the temporary file to the destination file path
  write_contents_to_destination_server(SS2, destination_path, source);
}

char *copy_directory(char *source, char *destination)
{
  /*
    Function called if the user wants to copy a directory
  */

  ValueStruct *SS1;
  ValueStruct *SS2;
  char *temp = (char *)malloc(1024);

  SS1 = find(accessible_paths_hashmap, source);
  SS2 = find(accessible_paths_hashmap, destination);

  // get the contents of the source folder
  char **contents = get_contents(accessible_paths_hashmap, source);
  char **dest_contents = get_dest_contents(accessible_paths_hashmap, source);
  int num_contents = 0;

  // print the contents of the folder
  for (int i = 0; contents[i] != NULL; i++)
  {
    // printf("%s\n", contents[i]);
    num_contents++;
  }

  // Append the destination path to the contents
  // printf(CYN "After appending the destination path : \n" reset);
  for (int i = 0; dest_contents[i] != NULL; i++)
  {
    strcpy(temp, destination);
    strcat(temp, dest_contents[i]);
    strcpy(dest_contents[i], temp);
    // printf("%s\n", dest_contents[i]);
  }
  sort_contents(contents, num_contents);
  sort_contents(dest_contents, num_contents);

  // printf(CYN "After sorting the source contents are : \n" reset);
  // for (int i = 0; contents[i] != NULL; i++)
  // {
  //   printf("%s\n", contents[i]);
  // }

  // printf(CYN "After sorting the destination contents are : \n" reset);
  // for (int i = 0; dest_contents[i] != NULL; i++)
  // {
  //   printf("%s\n", dest_contents[i]);
  // }

  // make the relevant files and folders on the destination paths
  for (int i = 0; dest_contents[i] != NULL; i++)
  {
    if (dest_contents[i][strlen(dest_contents[i]) - 1] == '/')
    {
      create_folder_in_destination_server(SS2, dest_contents[i]);
    }
    else
    {
      create_file_in_destination_server(SS2, dest_contents[i]);
      read_and_save_contents_from_source_server(SS1, contents[i]);
      write_contents_to_destination_server(SS2, dest_contents[i], contents[i]);
    }
  }

  return "Done copying directories";
}

char *copy(char *filename1, char *filename2)
{
  /*
    Main function which is called when the user wants to copy something
  */

  printf(GRN "Copying file %s to %s\n" reset, filename1, filename2);
  ValueStruct *myStruct;
  if ((myStruct = find(accessible_paths_hashmap, filename1)) == NULL)
  {
    printf(RED "File not found\n" reset);
    return RED "> NM : File not found" reset;
  }

  ValueStruct *myStruct2;
  if ((myStruct2 = find(accessible_paths_hashmap, filename2)) == NULL)
  {
    printf(RED "File not found\n" reset);
    return RED "> NM : File not found" reset;
  }

  // check if the filenam1 ends in a '/'
  if (filename1[strlen(filename1) - 1] != '/')
  {
    return copy_file(filename1, filename2);
  }
  else
  {
    return copy_directory(filename1, filename2);
  }

  return GRN "> NM : Copying the file" reset;
}

char *LS()
{
  /*
    Function to list all the accessible paths
  */

  char *keys = get_all_keys(accessible_paths_hashmap);
  // printf("Keys are: %s\n", keys);
  char *response = (char *)malloc(1000);
  strcpy(response, GRN "> NM : Listing all the accessible paths" reset);
  strcat(response, "\n");
  strcat(response, keys);
  return response;
}

#endif // OPERATION_HANDLER_H