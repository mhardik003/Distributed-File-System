#ifndef LRU_CACHING_H
#define LRU_CACHING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h" // Assuming this includes your hashmap implementation
#include "utils.h"

#define CACHE_SIZE 5

typedef struct Node
{
    /*
        Structure of a node in the linked list for the LRU cache
    */

    char key[256]; // Assuming keys are strings
    ValueStruct value;
    struct Node *prev, *next;
} Node;

typedef struct
{
    /*
        Structure of the LRU cache
    */

    Node *head, *tail;
    HashmapItem *hashmap[MAX_HASHMAP_SIZE];
    Node *cacheNodes[CACHE_SIZE];
    int size;
} LRUCache;

// Function Declarations
Node *createNode(const char *key, ValueStruct value);
void detachNode(LRUCache *cache, Node *node);
void attachNodeAtFront(LRUCache *cache, Node *node);
void evictIfNecessary(LRUCache *cache);
void removeFolderFromCache(LRUCache *cache, const char *folder);
void removeFileFromCache(LRUCache *cache, const char *fileKey);
int isFileInCache(LRUCache *cache, const char *fileKey);
void putInCache(LRUCache *cache, const char *key, ValueStruct value);
ValueStruct *getFromCache(LRUCache *cache, const char *key);
void cacheInit(LRUCache *cache);
void cacheCleanup(LRUCache *cache);
void printCache(const LRUCache *cache);

Node *createNode(const char *key, ValueStruct value)
{
    /*
        Creates a new node for the LRU cache
    */

    Node *node = (Node *)malloc(sizeof(Node));
    if (node == NULL)
    {
        perror("Error in creating a new node");
        exit(EXIT_FAILURE);
    }
    
    strcpy(node->key, key);
    node->value = value;
    node->prev = node->next = NULL;
    return node;
}

void detachNode(LRUCache *cache, Node *node)
{
    /*
        Detaches the given node from the linked list
    */

    if (node->prev)
    {
        node->prev->next = node->next;
    }
    else
    {
        cache->head = node->next;
    }
    if (node->next)
    {
        node->next->prev = node->prev;
    }
    else
    {
        cache->tail = node->prev;
    }
}

void attachNodeAtFront(LRUCache *cache, Node *node)
{

    /*
        Attaches the given node at the front of the linked list
        Used when a node is accessed or added to the cache
   */

    if (node->prev != NULL || node->next != NULL)
    {
        detachNode(cache, node);
    }

    // Now, attach it at the front
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head)
    {
        cache->head->prev = node;
    }
    cache->head = node;
    if (cache->tail == NULL)
    {
        cache->tail = node;
    }
}

void evictIfNecessary(LRUCache *cache)
{

    /*
        Evicts the least recently used item from the cache if the cache is full
    */

    if (cache->size >= CACHE_SIZE)
    {
        // Remove the least recently used (LRU) item
        Node *lru = cache->tail;
        detachNode(cache, lru);

        // Update cacheNodes array
        for (int i = 0; i < cache->size; i++)
        {
            if (cache->cacheNodes[i] == lru)
            {
                // Move all subsequent nodes one position to the left
                for (int j = i; j < cache->size - 1; j++)
                {
                    cache->cacheNodes[j] = cache->cacheNodes[j + 1];
                }
                break;
            }
        }

        // Free the node and update the size
        free(lru);
        cache->size--;
    }
}

void removeFolderFromCache(LRUCache *cache, const char *folder)
{
    /*
        Removes all the files in the given folder from the cache
        Used when a folder is deleted
    */

    Node *current = cache->head;
    while (current != NULL)
    {
        Node *temp = current;
        current = current->next;

        // Check if the key starts with the folder name
        if (strncmp(temp->key, folder, strlen(folder)) == 0)
        {
            // Detach the node from the linked list
            detachNode(cache, temp);
            cache->size--;

            // Free the node
            free(temp);
        }
    }

    // Call the remove_folder function from the hashmap
    remove_folder(cache->hashmap, folder);
}

void removeFileFromCache(LRUCache *cache, const char *fileKey)
{
    /*
        Removes the given file from the cache
        Used when a file is deleted
    */

    Node *current = cache->head;
    Node *toRemove = NULL;

    // Find the file in the cache
    while (current != NULL)
    {
        if (strcmp(current->key, fileKey) == 0)
        {
            toRemove = current;
            break;
        }
        current = current->next;
    }

    // If found, remove it from the cache
    if (toRemove)
    {
        if (toRemove->prev)
        {
            toRemove->prev->next = toRemove->next;
        }
        else
        {
            cache->head = toRemove->next;
        }

        if (toRemove->next)
        {
            toRemove->next->prev = toRemove->prev;
        }
        else
        {
            cache->tail = toRemove->prev;
        }

        free(toRemove);
        cache->size--;
    }

    // Now remove the file from the hashmap
    remove_key(cache->hashmap, fileKey);
}

int isFileInCache(LRUCache *cache, const char *fileKey)
{

    /*
        Checks if the given file is present in the cache
    */

    Node *current = cache->head;
    while (current != NULL)
    {
        if (strcmp(current->key, fileKey) == 0)
        {
            return 1;
        }
        current = current->next;
    }
    return 0;
}

void putInCache(LRUCache *cache, const char *key, ValueStruct value)
{

    /*
        Adds the given key-value pair of the Hashmap to the cache
    */

    Node *existingNode = NULL;

    // Check if the key already exists in the cache
    for (int i = 0; i < cache->size; i++)
    {
        if (strcmp(cache->cacheNodes[i]->key, key) == 0)
        {
            existingNode = cache->cacheNodes[i];
            break;
        }
    }

    if (existingNode)
    {
        // Update the value of the existing node
        existingNode->value = value;
        attachNodeAtFront(cache, existingNode);
    }
    else
    {
        // Check if we need to evict a node first
        evictIfNecessary(cache);

        // Create a new node and add it to the front of the cache
        Node *newNode = createNode(key, value);
        attachNodeAtFront(cache, newNode);

        // Add the new node to the cacheNodes array
        if (cache->size < CACHE_SIZE)
        {
            cache->cacheNodes[cache->size] = newNode;
            cache->size++;
        }

        // Insert into hashmap only if it doesn't already exist
        if (find(cache->hashmap, key) == NULL)
        {
            insert(cache->hashmap, key, value);
        }
    }
}

ValueStruct *getFromCache(LRUCache *cache, const char *key)
{
    /*
        Returns the value corresponding to the given key from the cache
    */

    for (int i = 0; i < cache->size; i++)
    {
        if (strcmp(cache->cacheNodes[i]->key, key) == 0)
        {
            puts("File found in cache");
            Node *node = cache->cacheNodes[i];
            detachNode(cache, node);
            attachNodeAtFront(cache, node);
            return &node->value;
        }
    }
    ValueStruct *value = find(cache->hashmap, key);
    if (value)
    {
        puts("File not found in cache");
        putInCache(cache, key, *value);
        return value;
    }
    return NULL;
}

void cacheInit(LRUCache *cache)
{
    /*
        Initializes the cache
    */

    cache->head = cache->tail = NULL;
    cache->size = 0;
    init_hashmap(cache->hashmap);
}

void cacheCleanup(LRUCache *cache)
{
    /*
        Frees all the memory used by the cache
    */

    Node *current = cache->head;
    while (current != NULL)
    {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    cleanup_hashmap(cache->hashmap);
}

void printCache(const LRUCache *cache)
{
    /*
        Prints the contents of the cache
    */

    Node *current = cache->head;
    printf("Cache Contents:\n");
    while (current != NULL)
    {
        printf("\t Key: %s\t|\tValue:\t{IP: %s, NM Port: %d, Client Port: %d, Num Readers: %d, Is Writing: %d}\n",
               current->key,
               current->value.ip,
               current->value.nm_port,
               current->value.client_port,
               current->value.num_readers,
               current->value.isWriting);
        current = current->next;
    }
}

#endif // LRU_CACHING_H