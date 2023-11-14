// ignore warnings, they are because of C++ malloc requirements which are not needed in C
#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Define the structure for a single hashmap entry
typedef struct hashmap_entry {
    char *key;
    char *value;
    struct hashmap_entry *next;
} hashmap_entry;

// Define the structure for the hashmap
typedef struct {
    hashmap_entry **entries;
    size_t size;
} hashmap;

// Function prototypes
hashmap *create_hashmap(size_t size);
void free_hashmap(hashmap *map);
bool hashmap_insert(hashmap *map, const char *key, const char *value);
char *hashmap_get(hashmap *map, const char *key);
bool hashmap_remove(hashmap *map, const char *key);

// Helper functions
unsigned long hash_string(const char *str, size_t size);
hashmap_entry *create_entry(const char *key, const char *value);

// Function implementations
hashmap *create_hashmap(size_t size) {
    hashmap *map = malloc(sizeof(hashmap));
    if (!map) return NULL;

    map->size = size;
    map->entries = malloc(sizeof(hashmap_entry*) * size);
    if (!map->entries) {
        free(map);
        return NULL;
    }

    for (size_t i = 0; i < size; i++) {
        map->entries[i] = NULL;
    }

    return map;
}

void free_hashmap(hashmap *map) {
    if (!map) return;

    for (size_t i = 0; i < map->size; i++) {
        hashmap_entry *entry = map->entries[i];
        while (entry) {
            hashmap_entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp->value);
            free(temp);
        }
    }

    free(map->entries);
    free(map);
}

bool hashmap_insert(hashmap *map, const char *key, const char *value) {
    unsigned long index = hash_string(key, map->size);
    hashmap_entry *new_entry = create_entry(key, value);

    if (!new_entry) return false;

    new_entry->next = map->entries[index];
    map->entries[index] = new_entry;
    return true;
}

char *hashmap_get(hashmap *map, const char *key) {
    unsigned long index = hash_string(key, map->size);
    hashmap_entry *entry = map->entries[index];

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

bool hashmap_remove(hashmap *map, const char *key) {
    unsigned long index = hash_string(key, map->size);
    hashmap_entry *entry = map->entries[index];
    hashmap_entry *prev_entry = NULL;

    while (entry) {
        if (strcmp(entry->key, key) == 0) {
            if (prev_entry) {
                prev_entry->next = entry->next;
            } else {
                map->entries[index] = entry->next;
            }
            free(entry->key);
            free(entry->value);
            free(entry);
            return true;
        }
        prev_entry = entry;
        entry = entry->next;
    }

    return false;
}

// Helper Functions
unsigned long hash_string(const char *str, size_t size) {
    unsigned long hash = 5381;
    int c;

    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }

    return hash % size;
}

hashmap_entry *create_entry(const char *key, const char *value) {
    hashmap_entry *entry = malloc(sizeof(hashmap_entry));
    if (!entry) return NULL;

    entry->key = strdup(key);
    if (!entry->key) {
        free(entry);
        return NULL;
    }

    entry->value = strdup(value);
    if (!entry->value) {
        free(entry->key);
        free(entry);
        return NULL;
    }

    entry->next = NULL;
    return entry;
}

#endif // HASHMAP_H


/*
Example usage:
// Create a hashmap
    hashmap *map = create_hashmap(10);

    // Insert key-value pairs into the hashmap
    hashmap_insert(map, "key1", "value1");
    hashmap_insert(map, "key2", "value2");

    // Retrieve and print a value from the hashmap
    char *value = hashmap_get(map, "key2");
    if (value != NULL) {
        printf("Value for 'key1': %s\n", value);
    } else {
        printf("Key not found.\n");
    }

    // Clean up
    free_hashmap(map);
*/