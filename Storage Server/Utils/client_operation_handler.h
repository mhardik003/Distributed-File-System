#ifndef OPERATION_HANDLER_H
#define OPERATION_HANDLER_H

#include <stdlib.h>
#include <string.h>
#include "server_setup.h"

void parseClientInput(char *input, int sock);
void operational_handler(char **input_tokens, int num_tokens, int sock);
void readFile(int sock, char **client_input_tokens);
void writeFile(int sock, char **client_input_tokens, int num_tokens);
void getInfo(int sock, char **client_input_tokens);

void parseClientInput(char *input, int sock)
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
    else
    {
        printf("Invalid command\n");
    }
}

void readFile(int sock, char **client_input_tokens)
{
    FILE *file;
    char *buffer;
    long fileLen;

    // Open the file in read mode
    printf("reading from the file $%s$\n", client_input_tokens[1]);
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
        fprintf(stderr, "Memory allocation failed!\n");
        fclose(file);
        return;
    }

    // Read file contents into buffer
    fread(buffer, fileLen, 1, file);
    fclose(file); // Close the file

    // Null-terminate the buffer
    buffer[fileLen] = '\0';

    // Print the contents
    printf("File contents:\n%s", buffer);
    sendMessage(sock, buffer);

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
        perror("Error getting file info");
        return;
    }

    strcat(message, "File Size: \n");
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
}

#endif