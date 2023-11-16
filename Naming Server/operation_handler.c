#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GRN "\e[0;32m"
#define RED "\e[0;31m"
#define reset "\e[0m"

char *create(char *filename)
{
    printf(GRN"Creating file %s"reset, filename);
    return GRN"Creating the file "reset;
}

char *delete(char *filename)
{

    printf(GRN"Deleting file %s"reset, filename);
    return GRN"Deleting the file"reset;
}

char *read(char *filename)
{
    printf(GRN"Reading file %s"reset, filename);
    // TODO-Mihika: search for ip and client_port of this filename in hashmap, return both of these to the client
    // then the client sends a TCP request to the ip:port
    // we need to modify the ss to keep listening for requests
    return GRN"Reading the file"reset;
}

char *write(char *filename, char *data)
{
    printf(GRN"Writing to file %s"reset, filename);
    return GRN"Writing to the file"reset;
}

char *copy(char *filename1, char *filename2)
{
    printf(GRN"Copying file %s to %s"reset, filename1, filename2);
    return GRN"Copying the file"reset;
}

char *get_info(char *filename)
{
    printf(GRN"Getting info of file %s"reset, filename);
    return GRN"Getting info of the file"reset;
}

char *LS(char *filename)
{
    printf(GRN"Listing files in directory %s"reset, filename);
    return GRN"Listing files in the directory"reset;
}

char *operation_handler(char **inputs, int num_inputs)
{
    if (num_inputs == 2)
    {
        if (strcmp(inputs[0], "CREATE") == 0)
        {
            return create(inputs[1]);
        }
        else if (strcmp(inputs[0], "DELETE") == 0)
        {
            return delete (inputs[1]);
        }
        else if (strcmp(inputs[0], "READ") == 0)
        {
            return read(inputs[1]);
        }
        else if (strcmp(inputs[0], "GETINFO") == 0)
        {
            return get_info(inputs[1]);
        }
        else if (strcmp(inputs[0], "LS") == 0)
        {
            return LS(inputs[1]);
        }
        else
        {
            printf(RED"Encountered invalid operation\n"reset);
            return RED"Invalid operation"reset;
        }
    }
    else if (num_inputs == 3)
    {
        if (strcmp(inputs[0], "WRITE") == 0)
        {
            return write(inputs[1], inputs[2]);
        }
        if (strcmp(inputs[0], "COPY") == 0)
        {
            return copy(inputs[1], inputs[2]);
        }
        else
        {
            printf(RED"Encountered invalid Operation\n"reset);
            return RED"Invalid operation"reset;
        }
    }
    else
    {
        printf(RED"Encountered invalid Operation\n"reset);
        return RED"Invalid operation"reset;
    }
}