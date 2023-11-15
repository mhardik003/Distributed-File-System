#ifndef GET_ACCESSIBLE_PATHS_H
#define GET_ACCESSIBLE_PATHS_H

#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>

int MAX_PATH_LENGTH = 4096;
int MAX_TOTAL_LENGTH = 1000000;

void listFilesRecursively(char *basePath, char **paths, int *length);
void getSelectedPaths(char *paths, char *selectedPaths);

void listFilesRecursively(char *basePath, char **paths, int *length) {
    char path[MAX_PATH_LENGTH];
    struct dirent *dp;
    struct stat statbuf;
    DIR *dir = opendir(basePath);

    if (!dir) {
        return;
    }

    while ((dp = readdir(dir)) != NULL) {
        if (strcmp(dp->d_name, ".") != 0 && strcmp(dp->d_name, "..") != 0) {
            snprintf(path, sizeof(path), "%s/%s", basePath, dp->d_name);

            if (stat(path, &statbuf) != -1) {
                snprintf(*paths + *length, MAX_TOTAL_LENGTH - *length, "%s\n", path);
                *length += strlen(path) + 1;
                if (S_ISDIR(statbuf.st_mode)) {
                    listFilesRecursively(path, paths, length);
                }
            }
        }
    }

    closedir(dir);
}

void getSelectedPaths(char *paths, char *selectedPaths) {
    int num, index;
    char *token;
    char buffer[MAX_TOTAL_LENGTH];

    printf("Enter -1 to select all paths, or the number of paths you would like to select: ");
    scanf("%d", &num);

    selectedPaths[0] = '\0';

    if (num == -1) {
        strcpy(selectedPaths, paths);
    } else {
        printf("Enter the indices of the paths you want to select:\n");
        for (int i = 0; i < num; i++) {
            scanf("%d", &index);
            strcpy(buffer, paths);
            token = strtok(buffer, "\n");

            for (int j = 1; token != NULL; j++) {
                if (j == index) {
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

#endif //GET_ACCESSIBLE_PATHS_H