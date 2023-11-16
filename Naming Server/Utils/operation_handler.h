#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define GRN "\e[0;32m"
#define RED "\e[0;31m"
#define reset "\e[0m"

char *create(char *filename) {
  printf(GRN "Creating file %s" reset, filename);
  return GRN "Creating the file " reset;
}

char *delete_file(char *filename) {

  printf(GRN "Deleting file %s\n" reset, filename);
  return GRN "Deleting the file" reset;
}

char *read_file(char *filename) {
  printf(GRN "Reading file %s\n" reset, filename);
  return GRN "Reading the file" reset;
}

char *write_file(char *filename, char *data) {
  printf(GRN "Writing '%s' to file %s\n" reset, data, filename);
  return GRN "Writing to the file" reset;
}

char *copy(char *filename1, char *filename2) {
  printf(GRN "Copying file %s to %s\n" reset, filename1, filename2);
  return GRN "Copying the file" reset;
}

char *get_info(char *filename) {
  printf(GRN "Getting info of file %s\n" reset, filename);
  return GRN "Getting info of the file" reset;
}

char *LS(char *filename) {
  printf(GRN "Listing files in directory %s\n" reset, filename);
  return GRN "Listing files in the directory" reset;
}

char *operation_handler(char **inputs, int num_inputs) {
  if (num_inputs == 2) {
    if (strcmp(inputs[0], "CREATE") == 0) {
      return create(inputs[1]);
    } else if (strcmp(inputs[0], "DELETE") == 0) {
      return delete_file(inputs[1]);
    } else if (strcmp(inputs[0], "READ") == 0) {
      return read_file(inputs[1]);
    } else if (strcmp(inputs[0], "GETINFO") == 0) {
      return get_info(inputs[1]);
    } else if (strcmp(inputs[0], "LS") == 0) {
      return LS(inputs[1]);
    } else {
      printf(RED "Encountered invalid operation\n" reset);
      return RED "Invalid operation" reset;
    }

  } else if (num_inputs == 3) {

    if (strcmp(inputs[0], "COPY") == 0) {
      return copy(inputs[1], inputs[2]);
    } else {
      printf(RED "Encountered invalid Operation\n" reset);
      return RED "Invalid operation" reset;
    }
  } else if (strcmp(inputs[0], "WRITE") == 0) {
    char *content = (char *)malloc(10000 * sizeof(char));
    for (int i = 2; i < num_inputs; i++) {
      strcat(content, inputs[i]);
      if (i != num_inputs - 1) {
        strcat(content, " ");
      }
    }
    return write_file(inputs[1], content);
  } else {
    printf(RED "Encountered invalid Operation\n" reset);
    return RED "Invalid operation" reset;
  }
}