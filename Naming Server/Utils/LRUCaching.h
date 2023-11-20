#ifndef LRU_CACHING_H
#define LRU_CACHING_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hashmap.h" // Assuming this includes your hashmap implementation
#include "utils.h"

#define CACHE_SIZE 5

typedef struct Node {
    char key[256];  // Assuming keys are strings
    ValueStruct value;
    struct Node *prev, *next;
} Node;

typedef struct {
    Node *head, *tail;
    HashmapItem *hashmap[MAX_HASHMAP_SIZE];
    Node *cacheNodes[CACHE_SIZE];
    int size;
} LRUCache;

Node* createNode(const char *key, ValueStruct value) {
    Node *node = (Node *)malloc(sizeof(Node));
    strcpy(node->key, key);
    node->value = value;
    node->prev = node->next = NULL;
    return node;
}

void detachNode(LRUCache *cache, Node *node) {
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        cache->head = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    } else {
        cache->tail = node->prev;
    }
}

void attachNodeAtFront(LRUCache *cache, Node *node) {
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) {
        cache->head->prev = node;
    }
    cache->head = node;
    if (cache->tail == NULL) {
        cache->tail = node;
    }
}

void evictIfNecessary(LRUCache *cache) {
    if (cache->size >= CACHE_SIZE) {
        // Remove the least recently used (LRU) item
        Node *lru = cache->tail;
        detachNode(cache, lru);
        // remove_key(cache->hashmap, lru->key); // Remove from hashmap
        free(lru);
        cache->size--;
    }
}

void removeFolderFromCache(LRUCache *cache, const char *folder) {
    Node *current = cache->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        
        // Check if the key starts with the folder name
        if (strncmp(temp->key, folder, strlen(folder)) == 0) {
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

void removeFileFromCache(LRUCache *cache, const char *fileKey) {
    Node *current = cache->head;
    Node *toRemove = NULL;

    // Find the file in the cache
    while (current != NULL) {
        if (strcmp(current->key, fileKey) == 0) {
            toRemove = current;
            break;
        }
        current = current->next;
    }

    // If found, remove it from the cache
    if (toRemove) {
        if (toRemove->prev) {
            toRemove->prev->next = toRemove->next;
        } else {
            cache->head = toRemove->next;
        }

        if (toRemove->next) {
            toRemove->next->prev = toRemove->prev;
        } else {
            cache->tail = toRemove->prev;
        }

        free(toRemove);
        cache->size--;
    }

    // Now remove the file from the hashmap
    remove_key(cache->hashmap, fileKey);
}

void putInCache(LRUCache *cache, const char *key, ValueStruct value) {
    Node *node = createNode(key, value);
    attachNodeAtFront(cache, node);
    cache->cacheNodes[cache->size++] = node;
    insert(cache->hashmap, key, value); // Insert into hashmap
    evictIfNecessary(cache);
}

ValueStruct* getFromCache(LRUCache *cache, const char *key) {
    for (int i = 0; i < cache->size; i++) {
        if (strcmp(cache->cacheNodes[i]->key, key) == 0) {
            puts("File found in cache");
            Node *node = cache->cacheNodes[i];
            detachNode(cache, node);
            attachNodeAtFront(cache, node);
            return &node->value;
        }
    }
    ValueStruct *value = find(cache->hashmap, key);
    if (value) {
        puts("File not found in cache");
        putInCache(cache, key, *value);
        return value;
    }
    return NULL;
}

void cacheInit(LRUCache *cache) {
    cache->head = cache->tail = NULL;
    cache->size = 0;
    init_hashmap(cache->hashmap);
}

void cacheCleanup(LRUCache *cache) {
    Node *current = cache->head;
    while (current != NULL) {
        Node *temp = current;
        current = current->next;
        free(temp);
    }
    cleanup_hashmap(cache->hashmap);
}

void printCache(const LRUCache *cache) {
    Node *current = cache->head;
    printf("Cache Contents:\n");
    while (current != NULL) {
        printf("Key: %s, Value: {IP: %s, NM Port: %d, Client Port: %d, Num Readers: %d, Is Writing: %d}\n",
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