#ifndef OPERATION_HANDLER_H
#define OPERATION_HANDLER_H

#include <stdlib.h>
#include <string.h>
#include "server_setup.h"

#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define CYN "\e[0;36m"
#define reset "\e[0m"

void parseInput(char *input, int sock);
void operational_handler(char **input_tokens, int num_tokens, int sock);
void readFile(int sock, char **client_input_tokens);
void writeFile(int sock, char **client_input_tokens, int num_tokens);
void getInfo(int sock, char **client_input_tokens);
void createFile(int sock, char **client_input_tokens);
void sendACK(char *message);

void parseInput(char *input, int sock)
{
    char **tokens = (char **)calloc(1000, sizeof(char *));
    for (int i = 0; i < 1000; i++)
    {
        tokens[i] = (char *)calloc(100, sizeof(char));
    }
    int num_tokens = 0;
    char *token = strtok(input, " ");
    while (token != NULL)
    {
        strcpy(tokens[num_tokens++], token);
        token = strtok(NULL, " ");
    }

    operational_handler(tokens, num_tokens, sock);
    free(tokens);
    return;
}

void operational_handler(char **input_tokens, int num_tokens, int sock)
{
    if (strcmp(input_tokens[0], "READ") == 0)
    {
        readFile(sock, input_tokens);
    }
    else if (strcmp(input_tokens[0], "WRITE") == 0)
    {
        writeFile(sock, input_tokens, num_tokens);
    }
    else if (strcmp(input_tokens[0], "GETINFO") == 0)
    {
        getInfo(sock, input_tokens);
    }
    else if (strcmp(input_tokens[0], "CREATE") == 0)
    {
        createFile(sock, input_tokens);
    }
    else
    {
        printf("Invalid command\n");
    }
}

void createFile(int sock, char **client_input_tokens)
{
    FILE *file;

    char *relativePath = (char *)calloc(1000, sizeof(char));
    char *file_name = (char *)calloc(1000, sizeof(char));

    // Find the last occurrence of '\'
    // relativePath = strrchr(client_input_tokens[1], '/');
    // file_name = strrchr(client_input_tokens[1], '/');
    // file_name = file_name + 1;

    // if (relativePath != NULL)
    // {
    //     *relativePath = '\0';
    // }

    // char fullPath[1024]; // Adjust the size as needed
    // fullPath[0] = '\0';
    // strcat(fullPath, relativePath);
    // strcat(fullPath, "/");
    // strcat(fullPath, file_name);
    // snprintf(fullPath, sizeof(fullPath), "%s%s", relativePath, file_name);

    // Create and open the file
    file = fopen(client_input_tokens[1], "w");
    if (file == NULL)
    {
        perror("Error creating file");
        sendMessage(sock, "-1");
        return;
    }

    printf("File '%s' created successfully.\n", client_input_tokens[1]);

    // Close the file
    fclose(file);
    sendMessage(sock, "CREATED");
    close(sock);

    return;
}

void readFile(int sock, char **client_input_tokens)
{
    FILE *file;
    char *buffer;
    long fileLen;

    // Open the file in read mode
    printf(YEL"reading from the file $%s$\n"reset, client_input_tokens[1]);
    file = fopen(client_input_tokens[1], "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Get the length of the file
    fseek(file, 0, SEEK_END);
    fileLen = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory for the buffer
    buffer = (char *)malloc(fileLen + 1);
    if (!buffer)
    {
        fprintf(stderr, RED"Memory allocation failed!\n"reset);
        fclose(file);
        return;
    }

    // Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    fclose(file); // Close the file

    // Null-terminate the buffer
    buffer[fileLen] = '\0';

    // Print the contents
    printf(GRN"Client < \n%s\n"reset, buffer);
    sendMessage(sock, buffer);
    close(sock);

    // send ACK to name server
    char *ackMessage = (char *)malloc(1000 * sizeof(char));
    ackMessage[0] = '\0';
    strcat(ackMessage, "ACK\nREAD\n");
    strcat(ackMessage, client_input_tokens[1]);
    sendACK(ackMessage);

    // Free the buffer memory
    free(buffer);
}

void getInfo(int sock, char **client_input_tokens)
{
    struct stat fileInfo;
    char *message = (char *)malloc(1000 * sizeof(char));
    message[0] = '\0';

    // Get file status
    if (stat(client_input_tokens[1], &fileInfo) != 0)
    {
        perror(RED"Error getting file info"reset);
        return;
    }

    strcat(message, "\nFile Size: ");
    char *size = (char *)malloc(100 * sizeof(char));
    sprintf(size, "%ld", fileInfo.st_size);
    strcat(message, size);
    strcat(message, " bytes ");
    strcat(message, "\n");
    strcat(message, "File Permissions: ");

    strcat(message, (S_ISDIR(fileInfo.st_mode)) ? "d" : "-");
    strcat(message, (fileInfo.st_mode & S_IRUSR) ? "r" : "-");
    strcat(message, (fileInfo.st_mode & S_IWUSR) ? "w" : "-");
    strcat(message, (fileInfo.st_mode & S_IXUSR) ? "x" : "-");
    strcat(message, (fileInfo.st_mode & S_IRGRP) ? "r" : "-");
    strcat(message, (fileInfo.st_mode & S_IWGRP) ? "w" : "-");
    strcat(message, (fileInfo.st_mode & S_IXGRP) ? "x" : "-");
    strcat(message, (fileInfo.st_mode & S_IROTH) ? "r" : "-");
    strcat(message, (fileInfo.st_mode & S_IWOTH) ? "w" : "-");
    strcat(message, (fileInfo.st_mode & S_IXOTH) ? "x" : "-");
    strcat(message, "\n");

    // Print last access time
    printf(CYN"Sending the following content to the client: \n"reset);
    printf("Last Accessed: %s", ctime(&fileInfo.st_atime));
    strcat(message, "Last Accessed: ");
    strcat(message, ctime(&fileInfo.st_atime));
    // strcat(message, "\n");

    // Print last modification time
    printf("Last Modified: %s", ctime(&fileInfo.st_mtime));
    strcat(message, "Last Modified: ");
    strcat(message, ctime(&fileInfo.st_mtime));
    // strcat(message, "\n");

    sendMessage(sock, message);
    close(sock);

    char *ackMessage = (char *)malloc(1000 * sizeof(char));
    ackMessage[0] = '\0';
    strcat(ackMessage, "ACK\nGETINFO\n");
    strcat(ackMessage, client_input_tokens[1]);
    sendACK(ackMessage);
    free(message);
}

void writeFile(int sock, char **client_input_tokens, int num_tokens)
{
    FILE *file;
    char *content = (char *)malloc(1000 * sizeof(char));
    content[0] = '\0';

    for (int i = 2; i < num_tokens; i++)
    {
        strcat(content, client_input_tokens[i]);
        strcat(content, " ");
    }
    printf("Writing to the file $%s$\n", client_input_tokens[1]);
    // Open the file in append mode
    file = fopen(client_input_tokens[1], "a");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Append the text to the file
    fputs(content, file);

    // Close the file
    fclose(file);

    printf("Text appended successfully.\n");
    sendMessage(sock, "Text appended successfully.\n");
    close(sock);

    // send ACK to naming server
    char *ackMessage = (char *)malloc(1000 * sizeof(char));
    ackMessage[0] = '\0';
    strcat(ackMessage, "ACK\nWRITE\n");
    strcat(ackMessage, client_input_tokens[1]);
    sendACK(ackMessage);

    free(content);
}

void sendACK(char *message) {
    int SS_NM_fd = createServerSocket();

    struct sockaddr_in SS_NM_connection;
    memset(&SS_NM_connection, 0, sizeof(SS_NM_connection));
    SS_NM_connection.sin_family = AF_INET;
    SS_NM_connection.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, name_server_ip, &SS_NM_connection.sin_addr) <= 0)
    { // Convert IPv4 and IPv6 addresses from text to binary form
        printf("\nInvalid address/ Address not supported \n");
        return;
    }

    connectToServer(SS_NM_fd, &SS_NM_connection);
    sendMessage(SS_NM_fd, message);
    close(SS_NM_fd);
}

#endif