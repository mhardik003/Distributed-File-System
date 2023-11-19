#ifndef GET_ACCESSIBLE_PATHS_H
#define GET_ACCESSIBLE_PATHS_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

#define RED "\e[0;31m"
#define GRN "\e[0;32m"
#define YEL "\e[0;33m"
#define BLU "\e[0;34m"
#define reset "\e[0m"

// #include "utils.h"

int MAX_PATH_LENGTH = 4096;
int MAX_TOTAL_LENGTH = 1000000;

void listFilesRecursively(char *basePath, char *currentPath, char **paths, int *length);
void getSelectedPaths(char *paths, char *selectedPaths);
void listFilesRecursively(char *basePath, char *currentPath, char **paths, int *length) {
    char path[MAX_PATH_LENGTH];
    struct dirent *dp;
    struct stat statbuf;
    DIR *dir = opendir(basePath);

    if (!dir) {
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            // Construct path correctly, avoiding double slashes
            if (strlen(currentPath) == 0) {
                snprintf(path, sizeof(path), "%s", dp->d_name);
            } else {
                snprintf(path, sizeof(path), "%s%s", currentPath, dp->d_name); // Removed the slash here
            }

            char fullPath[MAX_PATH_LENGTH];
            if (basePath[strlen(basePath) - 1] == '/') {
                snprintf(fullPath, sizeof(fullPath), "%s%s", basePath, dp->d_name); // Removed the slash here
            } else {
                snprintf(fullPath, sizeof(fullPath), "%s/%s", basePath, dp->d_name);
            }

            if (stat(fullPath, &statbuf) != -1) {
                if (S_ISDIR(statbuf.st_mode)) {
                    snprintf(*paths + *length, MAX_TOTAL_LENGTH - *length, "%s/\n", path);
                    *length += strlen(path) + 2; // +2 for '/' and '\n'
                } else {
                    snprintf(*paths + *length, MAX_TOTAL_LENGTH - *length, "%s\n", path);
                    *length += strlen(path) + 1; // +1 for '\n'
                }

                if (S_ISDIR(statbuf.st_mode)) {
                    char newCurrentPath[MAX_PATH_LENGTH];
                    snprintf(newCurrentPath, sizeof(newCurrentPath), "%s/", path); // Append '/' for recursive call
                    listFilesRecursively(fullPath, newCurrentPath, paths, length);
                }
            }
        }
    }

    closedir(dir);
}

void getSelectedPaths(char *paths, char *selectedPaths)
{
    int num, index;
    char *token;
    char buffer[MAX_TOTAL_LENGTH];

    printf(BLU "Enter -1 to select all paths, or the number of paths you would like to select: " reset);
    scanf("%d", &num);
    getchar();

    selectedPaths[0] = '\0';

    if (num == -1)
    {
        strcpy(selectedPaths, paths);
    }
    else
    {
        printf(BLU "Enter the indices of the paths you want to select:\n" reset);
        for (int i = 0; i < num; i++)
        {
            scanf("%d", &index);
            strcpy(buffer, paths);
            token = strtok(buffer, "\n");

            for (int j = 1; token != NULL; j++)
            {
                if (j == index)
                {
                    strcat(selectedPaths, token);
                    strcat(selectedPaths, "\n");
                    break;
                }
                token = strtok(NULL, "\n");
            }
        }
        getchar();
    }
}

#endif // GET_ACCESSIBLE_PATHS_H