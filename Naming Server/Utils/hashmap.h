#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Define the structure of the value
typedef struct
{
    char *ip;
    int nm_port;
    int client_port;
    int num_readers;
    int isWriting;
} ValueStruct;

typedef struct HashmapItem
{
    char *key;
    ValueStruct value;
    struct HashmapItem *next; // for handling collisions
} HashmapItem;

// Function Declarations

#define MAX_HASHMAP_SIZE 1000

// Hash function
unsigned int hash(const char *key)
{
    unsigned long int value = 0;
    unsigned int i = 0;
    unsigned int key_len = strlen(key);

    for (; i < key_len; ++i)
    {
        value = value * 37 + key[i];
    }

    return value % HASH_MAP_SIZE;
}

// Initialize the hashmap
void init_hashmap(HashmapItem *hashmap[])
{
    for (int i = 0; i < HASH_MAP_SIZE; ++i)
    {
        hashmap[i] = NULL;
    }
}

// Cleanup the hashmap
void cleanup_hashmap(HashmapItem *hashmap[])
{
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

// Insert function
void insert(HashmapItem *hashmap[], const char *key, ValueStruct value)
{
    unsigned int index = hash(key);
    HashmapItem *newItem = (HashmapItem *)malloc(sizeof(HashmapItem));
    newItem->key = strdup(key);
    newItem->value = value;
    newItem->next = hashmap[index];
    hashmap[index] = newItem;
}

// Find function
ValueStruct *find(HashmapItem *hashmap[], const char *key)
{
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

// Remove function
void remove_key(HashmapItem *hashmap[], const char *key)
{
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
    // remove all the keys that start with the folder
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
    // concatenate all the keys and return
    char *all_keys = (char *)malloc(MAX_HASHMAP_SIZE * 100);
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
    // get all the keys that start with the folder
    char **contents = (char **)malloc(MAX_HASHMAP_SIZE * sizeof(char *));
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
    // get all the keys that start with the folder
    char **contents = (char **)malloc(MAX_HASHMAP_SIZE * sizeof(char *));
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
    // sort the contents
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

#endif // HASHMAP_H

/*
Example Usage:
int main() {
    init_hashmap(hashmap);

    // Example usage
    ValueStruct vs = {"Value1", 42, "Value3"};
    insert(hashmap, "key1", vs);

    ValueStruct* found = find(hashmap, "key1");
    if (found != NULL) {
        printf("Found: %s, %d, %s\n", found->value1, found->value2, found->value3);
    }

    remove_key(hashmap, "key1");
    found = find(hashmap, "key1");
    if (found == NULL) {
        printf("Key1 not found\n");
    }

    cleanup_hashmap(hashmap);
    return 0;
}
*/