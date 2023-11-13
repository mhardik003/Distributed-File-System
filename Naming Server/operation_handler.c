char *create(char *filename)
{
    return "Creating the file"+" "+filename+"\n";
}

char *delete (char *filename)
{
    return "Deleting the file"+" "+filename+"\n";
}

char *read(char *filename)
{
    return "Reading the file"+" "+filename+"\n";
}

char *write(char *filename, char *data)
{
    return "Writing to the file"+" "+filename+"\n";
}

char *copy(char *filename1, char *filename2)
{
    return "Copying the file"+" "+filename1+" "+"to"+" "+filename2+"\n";
}

char *get_info(char *filename)
{
    return "Getting info of the file"+" "+filename+"\n";
}

char *LS(char *filename)
{
    return "Listing the files in the directory"+" "+filename+"\n";
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
        else if ((strcmp(inupts[0]), "GETINFO") == 0)
        {
            return get_info(inputs[1]);
        }
        else if ((strcmp(inputs[0]), "LS") == 0)
        {
            return LS(inputs[1]);
        }
        else
        {
            return "Invalid operation\n";
        }
    }
    else if (num_input == 3)
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
            return "Invalid operation\n";
        }
    }
    else
    {
        return "Invalid operation\n";
    }
}