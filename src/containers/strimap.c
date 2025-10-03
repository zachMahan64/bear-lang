#include "strimap.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

strimap_t strimap_create(size_t capacity) {
    strimap_t map;
    map.capacity = capacity;
    map.buckets = (strimap_entry_t**)calloc(capacity,
                                            sizeof(strimap_entry_t*)); // init to NULL
    return map;
}
void strimap_destroy(strimap_t* map) {
    if (!map || !map->buckets) {
        return;
    }
    for (size_t i = 0; i < map->capacity; i++) {
        strimap_entry_t* curr = map->buckets[i];
        while (curr) {
            strimap_entry_t* next = curr->next;
            free(curr->key); // since key string was malloc'd
            free(curr);
            curr = next;
        }
    }
    free((void*)map->buckets); // free buckets themselves
    map->buckets = NULL;
    map->size = 0;
    map->capacity = 0;
}

void strimap_insert(strimap_t* map, char* key, int val) {
    // TODO
}

void strimap_rehash(strimap_t* map, size_t new_capacity) {
    strimap_t temp = strimap_create(new_capacity);

    for (size_t i = 0; i < map->capacity; i++) {
        strimap_entry_t* curr = map->buckets[i];
        while (curr) {
            strimap_non_rehashing_insert(&temp, curr->key, curr->val);

            curr = curr->next;
            // TODO finish
        }
    }
    // TODO
}

// helpers
uint64_t hash_string(const char* str) {
    uint64_t hash = 5381;
    int curr_ch;

    while ((curr_ch = (unsigned char)*str++)) { // iter
        // hash = hash * 33 + c (DJB2 algorithm)
        hash = ((hash << 5) + hash) + curr_ch;
    }

    return hash;
}

void strimap_non_rehashing_insert(strimap_t* map, char* key, int val) {
    uint64_t raw_hash = hash_string(key);
    uint64_t bucket_idx = raw_hash % map->capacity;

    strimap_entry_t* curr = map->buckets[bucket_idx];

    while (curr) {
        if (strcmp(key, curr->key) == 0) {
            curr->val = val;
            return; // if key already exists, update val
        }
        if (!curr->next) {
            strimap_entry_t* entry = malloc(sizeof(strimap_entry_t));
            size_t key_size = strlen(key) + 1;
            char* fresh_key = malloc(key_size);
            memcpy(fresh_key, key, key_size);
            entry->key = fresh_key;
            entry->val = val;
            entry->next = NULL;
            curr->next = entry;
            return; // add key to end of chain
        }
        curr = curr->next;
    }
    // if we skipped traverse loop because curr bucket is null, just insert straight into the bucket
    strimap_entry_t* entry = malloc(sizeof(strimap_entry_t));
    size_t key_size = strlen(key) + 1;
    char* fresh_key = malloc(key_size);
    memcpy(fresh_key, key, key_size);
    entry->key = fresh_key;
    entry->val = val;
    entry->next = NULL;
    map->buckets[bucket_idx] = entry;
}

// TODO finish impl
