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
int endsWithSlash(const char *str);
void createFile(int sock, char **client_input_tokens);
void createFolder(int sock, char **input_tokens);
void deleteFolder(char *path);
void deleteFile(char *path);
void deleteFunction(int sock, char **client_input_tokens);
void NMreadFile(int sock, char **NM_input_tokens);
void readFile(int sock, char **client_input_tokens);
void NMwriteFile(int sock, char **client_input_tokens, int num_tokens);
void writeFile(int sock, char **client_input_tokens, int num_tokens);
void getInfo(int sock, char **client_input_tokens);
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
        return;
    }
    if (strcmp(input_tokens[0], "NMREAD") == 0)
    {
        NMreadFile(sock, input_tokens);
        return;
    }
    else if (strcmp(input_tokens[0], "WRITE") == 0)
    {
        writeFile(sock, input_tokens, num_tokens);
        return;
    }
    else if (strcmp(input_tokens[0], "NMWRITE") == 0)
    {
        NMwriteFile(sock, input_tokens, num_tokens);
        return;
    }
    else if (strcmp(input_tokens[0], "GETINFO") == 0)
    {
        getInfo(sock, input_tokens);
        return;
    }
    else if (strcmp(input_tokens[0], "CREATE") == 0)
    {
        createFile(sock, input_tokens);
        return;
    }
    else if (strcmp(input_tokens[0], "CREATEFOLDER") == 0)
    {
        createFolder(sock, input_tokens);
        return;
    }
    else if (strcmp(input_tokens[0], "DELETE") == 0)
    {
        deleteFunction(sock, input_tokens);
        return;
    }
    else
    {
        printf("Invalid command\n");
        return;
    }
}

int endsWithSlash(const char *str)
{
    size_t len = strlen(str);
    return (len > 0 && str[len - 1] == '/') ? 1 : 0;
}

void createFile(int sock, char **client_input_tokens)
{
    FILE *file;

    char *relativePath = (char *)calloc(1000, sizeof(char));
    char *file_name = (char *)calloc(1000, sizeof(char));

    // check if the client_input_tokens[1] ends in a '/' or not

    if (endsWithSlash(client_input_tokens[1])) // make a directory
    {
        // check if the directory already exists
        if (access(client_input_tokens[1], F_OK) != -1)
        {
            printf(RED "Directory '%s' already exists.\n" reset, client_input_tokens[1]);
            sendMessage(sock, "-1");
            close(sock);

            return;
        }

        if (mkdir(client_input_tokens[1], 0777) == -1)
        {
            perror(RED "Error creating directory" reset);
            sendMessage(sock, "-1");
            close(sock);

            return;
        }
        printf(GRN "Directory '%s' created successfully.\n" reset, client_input_tokens[1]);
        sendMessage(sock, "CREATED");
        close(sock);
        return;
    }
    else // make a file
    {
        // check if the file already exists
        if (access(client_input_tokens[1], F_OK) != -1)
        {
            printf(RED "File '%s' already exists.\n" reset, client_input_tokens[1]);
            sendMessage(sock, "-1");
            close(sock);

            return;
        }
        // Create and open the file
        file = fopen(client_input_tokens[1], "w");
        if (file == NULL)
        {
            perror(RED "Error creating file" reset);
            sendMessage(sock, "-1");
            close(sock);

            return;
        }
        printf(GRN "File '%s' created successfully.\n" reset, client_input_tokens[1]);

        // Close the file
        fclose(file);
        sendMessage(sock, "CREATED");
        close(sock);
        return;
    }
}

void createFolder(int sock, char **client_input_tokens)
{
    // check if the client_input_tokens[1] ends in a '/' or not

    // check if the directory already exists
    printf("Making a folder %s\n", client_input_tokens[1]);
    if (access(client_input_tokens[1], F_OK) != -1)
    {
        printf(RED "Directory '%s' already exists.\n" reset, client_input_tokens[1]);
        sendMessage(sock, "-1");
        close(sock);

        return;
    }

    if (mkdir(client_input_tokens[1], 0777) == -1)
    {
        perror(RED "Error creating directory" reset);
        sendMessage(sock, "-1");
        close(sock);

        return;
    }
    printf(GRN "Directory '%s' created successfully.\n" reset, client_input_tokens[1]);
    sendMessage(sock, "CREATED");
    close(sock);
    return;
}

void deleteFolder(char *path)
{
    /*
        Function to delete a folder and its contents recursively
    */

    DIR *d = opendir(path);
    if (d)
    {
        struct dirent *dir;
        while ((dir = readdir(d)) != NULL)
        {
            if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..") != 0)
            {
                char *file_path = (char *)malloc(strlen(path) + strlen(dir->d_name) + 2);
                sprintf(file_path, "%s/%s", path, dir->d_name);

                struct stat statbuf;
                if (!stat(file_path, &statbuf))
                {
                    if (S_ISDIR(statbuf.st_mode))
                        deleteFolder(file_path);
                    else
                        remove(file_path);
                }
                free(file_path);
            }
        }
        closedir(d);
    }
    rmdir(path);
}

void deleteFile(char *path)
{
    if (remove(path) == 0)
    {
        printf(GRN "File '%s' deleted successfully.\n" reset, path);
    }
    else
    {
        perror("Error deleting file");
    }
}

void deleteFunction(int sock, char **client_input_tokens)
{
    // check if the input is a file or a folder
    if (endsWithSlash(client_input_tokens[1]))
    {
        deleteFolder(client_input_tokens[1]);
        sendMessage(sock, "DELETED FOLDER");
        close(sock);
    }
    else
    {
        deleteFile(client_input_tokens[1]);
        sendMessage(sock, "DELETED FILE");
        close(sock);
    }
}

void NMreadFile(int sock, char **NM_input_tokens)
{
    /*
        Function to read a file by the Naming Server
    */
    FILE *file;
    char *buffer;
    long fileLen;

    // Open the file in read mode
    file = fopen(NM_input_tokens[1], "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Allocate memory for the buffer
    size_t bufferSize = 1024;
    buffer = (char *)malloc(bufferSize);

    if (!buffer)
    {
        fprintf(stderr, RED "Memory allocation failed!\n" reset);
        fclose(file);
        return;
    }

    // read the contents from the  file and store it in the buffer variable
    long bytesRead = 0;
    bytesRead = fread(buffer, 1, bufferSize, file);
    // printf("Contents of the file : %s\n", buffer);

    // Read file contents into buffer
    sendMessage(sock, buffer); // Send the chunk

    fclose(file); // Close the file
    close(sock);
    // Print the contents
    printf(GRN "SS > Sent the following contents to the NM! : %s \n" reset, buffer);
}

void readFile(int sock, char **client_input_tokens)
{
    /*
        Function to read a file by the client
    */

    FILE *file;
    char *buffer;
    long fileLen;

    // Open the file in read mode
    printf(YEL "reading from the file '%s'\n" reset, client_input_tokens[1]);
    file = fopen(client_input_tokens[1], "rb");
    if (file == NULL)
    {
        perror("Error opening file");
        return;
    }

    // Allocate memory for the buffer
    size_t bufferSize = 512;
    buffer = (char *)malloc(bufferSize);

    if (!buffer)
    {
        fprintf(stderr, RED "Memory allocation failed!\n" reset);
        fclose(file);
        return;
    }

    long bytesRead = 0;
    // Read file contents into buffer
    while ((bytesRead = fread(buffer, 1, bufferSize, file)) > 0)
    {
        buffer[bytesRead] = '\0';
        sendMessage(sock, buffer); // Send the chunk
    }
    fclose(file); // Close the file

    sendMessage(sock, "END"); // Send the end of file message
    close(sock);
    // Print the contents
    printf(GRN "SS > Sent the contents to the client! \n" reset);
    // sendMessage(sock, buffer);

    // send ACK to name server
    char *ackMessage = (char *)malloc(1000 * sizeof(char));
    ackMessage[0] = '\0';
    strcat(ackMessage, "ACK\nREAD\n");
    strcat(ackMessage, client_input_tokens[1]);
    sendACK(ackMessage);

    // Free the buffer memory
    free(buffer);
}

void NMwriteFile(int sock, char **client_input_tokens, int num_tokens)
{
    /*
        Function to write to a file by the Naming Server
    */

    FILE *file;
    char *content = (char *)malloc(1024 * sizeof(char));
    memset(content, 0, 1024);

    for (int i = 2; i < num_tokens; i++)
    {
        strcat(content, " ");
        strcat(content, client_input_tokens[i]);
    }

    printf("Writing to the file '%s'\n", client_input_tokens[1]);
    // Open the file in append mode
    file = fopen(client_input_tokens[1], "wb");

    if (file == NULL)
    {
        perror("Error opening file");
        close(sock);
        free(content);
        return;
    }
    // Append the text to the file
    fputs(content, file);

    // Close the file
    sendMessage(sock, "WRITTEN");

    fclose(file);
    close(sock);
    free(content);

    printf(GRN "Text written successfully.\n" reset);
}

void writeFile(int sock, char **client_input_tokens, int num_tokens)
{
    /*
        Function to write to a file
    */

    FILE *file;
    char *content = (char *)malloc(1000 * sizeof(char));
    content[0] = '\0';

    for (int i = 2; i < num_tokens; i++)
    {
        strcat(content, " ");
        strcat(content, client_input_tokens[i]);
    }
    printf("Writing to the file '%s'\n", client_input_tokens[1]);
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
    sendMessage(sock, "Text appended successfully.");
    close(sock);

    // send ACK to naming server
    char *ackMessage = (char *)malloc(1000 * sizeof(char));
    ackMessage[0] = '\0';
    strcat(ackMessage, "ACK\nWRITE\n");
    strcat(ackMessage, client_input_tokens[1]);
    sendACK(ackMessage);

    free(content);
}

void getInfo(int sock, char **client_input_tokens)
{
    /*
        Function to get the file/folder information and send it to the client
    */

    struct stat fileInfo;
    char *message = (char *)malloc(1000 * sizeof(char));
    message[0] = '\0';

    // Get file status
    if (stat(client_input_tokens[1], &fileInfo) != 0)
    {
        perror(RED "Error getting file info" reset);
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
    printf(CYN "Sending the following content to the client: \n" reset);
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

void sendACK(char *message)
{
    /*
        Function to send acknowledgement to the name server after the READ/WRITE/GETINFO operation is done
    */

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