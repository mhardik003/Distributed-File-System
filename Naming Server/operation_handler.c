#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *create(char *filename)
{
    printf("Creating file %s\n", filename);
    return "Creating the file ";
}

char *delete(char *filename)
{

    printf("Deleting file %s\n", filename);
    return "Deleting the file";
}

char *read(char *filename)
{
    printf("Reading file %s\n", filename);
    return "Reading the file";
}

char *write(char *filename, char *data)
{
    printf("Writing to file %s\n", filename);
    return "Writing to the file";
}

char *copy(char *filename1, char *filename2)
{
    printf("Copying file %s to %s\n", filename1, filename2);
    return "Copying the file";
}

char *get_info(char *filename)
{
    printf("Getting info of file %s\n", filename);
    return "Getting info of the file";
}

char *LS(char *filename)
{
    printf("Listing files in directory %s\n", filename);
    return "Listing files in the directory";
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
            printf("Encountered invalid operation\n");
            return "Invalid operation\n";
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
            printf("Encountered invalid Operation\n");
            return "Invalid operation\n";
        }
    }
    else
    {
        printf("Encountered invalid Operation\n");
        return "Invalid operation\n";
    }
}