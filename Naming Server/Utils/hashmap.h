#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "utils.h"

#define MAX_HASHMAP_SIZE 1000

// Define the structure of the value
typedef struct
{
    /*
        Structure of the value stored in the hashmap
    */

    char *ip;
    int nm_port;
    int client_port;
    int num_readers;
    int isWriting;
    sem_t write_semaphore;
    sem_t read_semaphore;
} ValueStruct;

typedef struct HashmapItem
{
    char *key;
    ValueStruct value;
    struct HashmapItem *next; // for handling collisions
} HashmapItem;

// Function Declarations
unsigned int hash(const char *key);
void init_hashmap(HashmapItem *hashmap[]);
void cleanup_hashmap(HashmapItem *hashmap[]);
void insert(HashmapItem *hashmap[], const char *key, ValueStruct value);
ValueStruct *find(HashmapItem *hashmap[], const char *key);
void remove_key(HashmapItem *hashmap[], const char *key);
void remove_folder(HashmapItem *hashmap[], const char *folder);
char *get_all_keys(HashmapItem *hashmap[]);
char **get_dest_contents(HashmapItem *hashmap[], char *folder);
char **get_contents(HashmapItem *hashmap[], char *folder);
void sort_contents(char **contents, int num_contents);
void remove_by_ip(HashmapItem *hashmap[], const char *ip);
char **find_by_ip(HashmapItem *hashmap[], const char *ip, int port);
void print_hashmap(HashmapItem *hashmap[]);

unsigned int hash(const char *key)
{
    /*
        Hash function used to hash the key
    */

    unsigned long int value = 0;
    unsigned int i = 0;
    unsigned int key_len = strlen(key);

    for (; i < key_len; ++i)
    {
        value = value * 37 + key[i];
    }

    return value % HASH_MAP_SIZE;
}

void init_hashmap(HashmapItem *hashmap[])
{
    /*
        Initialize the hashmap
    */

    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        hashmap[i] = NULL;
    }
}

void cleanup_hashmap(HashmapItem *hashmap[])
{
    /*
        Cleanup the hashmap by freeing all the memory
    */

    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *item = hashmap[i];
        while (item != NULL)
        {
            HashmapItem *temp = item;
            item = item->next;
            free(temp->key);
            free(temp);
        }
    }
}

void insert(HashmapItem *hashmap[], const char *key, ValueStruct value)
{
    /*
        Function to insert a key-value pair in the hashmap
    */

    unsigned int index = hash(key);
    HashmapItem *newItem = (HashmapItem *)malloc(sizeof(HashmapItem));
    if (newItem == NULL)
    {
        perror("Memory allocation failed");
        return;
    }

    newItem->key = strdup(key);
    newItem->value = value;
    newItem->next = hashmap[index];
    hashmap[index] = newItem;
}

ValueStruct *find(HashmapItem *hashmap[], const char *key)
{
    /*
        Function to find a key in the hashmap and return its value
    */

    unsigned int index = hash(key);
    HashmapItem *item = hashmap[index];
    while (item != NULL)
    {
        if (strcmp(item->key, key) == 0)
        {
            return &item->value;
        }
        item = item->next;
    }
    return NULL;
}

void remove_key(HashmapItem *hashmap[], const char *key)
{
    /*
        Function to remove a key from the hashmap
    */

    unsigned int index = hash(key);
    HashmapItem *current = hashmap[index];
    HashmapItem *prev = NULL;
    while (current != NULL)
    {
        if (strcmp(current->key, key) == 0)
        {
            if (prev == NULL)
            {
                hashmap[index] = current->next;
            }
            else
            {
                prev->next = current->next;
            }
            free(current->key);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void remove_folder(HashmapItem *hashmap[], const char *folder)
{
    /*
     Function to remove all the keys starting with the given folder
     As all the contents in the folder will have the same starting characters as the folder name
    */

    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *item = hashmap[i];
        while (item != NULL)
        {
            // find out if the key starts with the folder
            if (strncmp(item->key, folder, strlen(folder)) == 0)
            {
                remove_key(hashmap, item->key);
            }
            item = item->next;
        }
    }
}

char *get_all_keys(HashmapItem *hashmap[])
{
    /*
        Function to get all the keys in the hashmap
    */

    char *all_keys = (char *)malloc(MAX_HASHMAP_SIZE * 100);

    if (all_keys == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    all_keys[0] = '\0';
    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *item = hashmap[i];
        while (item != NULL)
        {
            strcat(all_keys, item->key);
            strcat(all_keys, "\n");
            item = item->next;
        }
    }
    return all_keys;
}

char **get_dest_contents(HashmapItem *hashmap[], char *folder)
{
    /*
        Used when we want to get the contents of a folder
        We get all the keys that start with the folder
        and then remove the folder name from the start of the key
    */

    char **contents = (char **)malloc(MAX_HASHMAP_SIZE * sizeof(char *));
    if (contents == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    int num_contents = 0;
    size_t folder_len = strlen(folder);
    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *item = hashmap[i];
        while (item != NULL)
        {
            // find out if the key starts with the folder
            if (strncmp(item->key, folder, strlen(folder)) == 0 && strcmp(item->key, folder) != 0)
            {
                contents[num_contents] = (char *)malloc(MAX_HASHMAP_SIZE * sizeof(char));
                if (contents[num_contents] == NULL)
                {
                    perror("Memory allocation failed");
                    return NULL;
                }
                strcpy(contents[num_contents], item->key + folder_len);
                num_contents++;
            }
            item = item->next;
        }
    }
    return contents;
}

char **get_contents(HashmapItem *hashmap[], char *folder)
{
    /*
        Used when we want to get the contents of a folder
        We get all the keys that start with the folder
    */
    char **contents = (char **)malloc(MAX_HASHMAP_SIZE * sizeof(char *));
    if (contents == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }

    int num_contents = 0;
    size_t folder_len = strlen(folder);
    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *item = hashmap[i];
        while (item != NULL)
        {
            // find out if the key starts with the folder
            if (strncmp(item->key, folder, strlen(folder)) == 0 && strcmp(item->key, folder) != 0)
            {
                contents[num_contents] = (char *)malloc(MAX_HASHMAP_SIZE * sizeof(char));
                if (contents[num_contents] == NULL)
                {
                    perror("Memory allocation failed");
                    return NULL;
                }
                strcpy(contents[num_contents], item->key);
                num_contents++;
            }
            item = item->next;
        }
    }
    return contents;
}

void sort_contents(char **contents, int num_contents)
{
    /*
        Sorts the contents of a folder in alphabetical order
    */

    for (int i = 0; i < num_contents; ++i)
    {
        for (int j = i + 1; j < num_contents; ++j)
        {
            if (strcmp(contents[i], contents[j]) > 0)
            {
                char *temp = contents[i];
                contents[i] = contents[j];
                contents[j] = temp;
            }
        }
    }
}

void remove_by_ip(HashmapItem *hashmap[], const char *ip)
{
    /*
        Used when we find out that a server is dead,
        then we remove all the entries related to it
    */

    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *current = hashmap[i];
        HashmapItem *prev = NULL;

        while (current != NULL)
        {
            // Store the next item before potentially freeing current
            HashmapItem *next = current->next;

            if (strcmp(current->value.ip, ip) == 0)
            {
                if (prev == NULL)
                {
                    // Remove the first item in the linked list
                    hashmap[i] = next;
                }
                else
                {
                    // Bypass the current item in the linked list
                    prev->next = next;
                }

                // Free the current item
                free(current->key);
                free(current);
            }
            else
            {
                // Move prev only if we didn't remove the current item
                prev = current;
            }

            // Move to the next item
            current = next;
        }
    }
}

char **find_by_ip(HashmapItem *hashmap[], const char *ip, int port)
{
    /*
        Finds all the key-value pairs having the given ip
        and returns them as a array of strings
    */

    char **keys = (char **)malloc(MAX_HASHMAP_SIZE * sizeof(char *));
    if (keys == NULL)
    {
        perror("Memory allocation failed");
        return NULL;
    }
    int num_keys = 0;

    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *item = hashmap[i];
        while (item != NULL)
        {
            if (strcmp(item->value.ip, ip) == 0 && item->value.nm_port == port)
            {
                keys[num_keys] = (char *)malloc(MAX_HASHMAP_SIZE * sizeof(char));
                if (keys[num_keys] == NULL)
                {
                    perror("Memory allocation failed");
                    return NULL;
                }
                strcpy(keys[num_keys], item->key);
                num_keys++;
            }
            item = item->next;
        }
    }
    return keys;
}

void print_hashmap(HashmapItem *hashmap[])
{
    /*
        Function to print all the key-value pairs in the hashmap
    */

    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        HashmapItem *item = hashmap[i];
        while (item != NULL)
        {
            printf("%s : %s %d %d\n", item->key, item->value.ip, item->value.nm_port, item->value.client_port);
            item = item->next;
        }
    }
}

#endif // HASHMAP_H
