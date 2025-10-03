#include "strimap.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// expects either a capacity or NULL for default capacity
strimap_t strimap_create(size_t capacity) {
    capacity = (capacity > STRIMAP_MINIMUM_CAPACITY) ? capacity : STRIMAP_MINIMUM_CAPACITY;
    strimap_t map;
    map.capacity = capacity;
    map.buckets = (strimap_entry_t**)calloc(capacity,
                                            sizeof(strimap_entry_t*)); // init to NULL
    map.size = 0;
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

void strimap_insert(strimap_t* map, const char* key, int val) {
    if ((double)map->size / (double)map->capacity >= STRIMAP_LOAD_FACTOR) {
        strimap_rehash(map, 2 * map->capacity);
    }
    uint64_t raw_hash = hash_string(key);
    uint64_t bucket_idx = raw_hash % map->capacity;

    strimap_entry_t* curr = map->buckets[bucket_idx];

    while (curr) {
        if (strcmp(key, curr->key) == 0) {
            curr->val = val;
            return; // if key already exists, update val
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
    entry->next = map->buckets[bucket_idx];
    map->buckets[bucket_idx] = entry;
    ++map->size;
}

void strimap_remove(strimap_t* map, const char* key) {
    uint64_t raw_hash = hash_string(key);
    uint64_t bucket_idx = raw_hash % map->capacity;

    strimap_entry_t* curr = map->buckets[bucket_idx];
    strimap_entry_t* prev = NULL;
    while (curr) {
        if (strcmp(key, curr->key) == 0) {
            if (prev) {
                prev->next = curr->next; // stitch chain around removed node
            } else {
                map->buckets[bucket_idx] = curr->next; // update head
            }
            free(curr->key); // since key string was malloc'd
            free(curr);
            --map->size;
            return; // if key already exists remove
        }
        prev = curr;
        curr = curr->next;
    }
    // if key not found, do nothing
}

int* strimap_at(strimap_t* map, const char* key) {
    uint64_t raw_hash = hash_string(key);
    uint64_t bucket_idx = raw_hash % map->capacity;

    strimap_entry_t* curr = map->buckets[bucket_idx];

    while (curr) {
        if (strcmp(key, curr->key) == 0) {
            return &curr->val;
        }
        curr = curr->next;
    }
    return NULL; // not found
}

const int* strimap_view(const strimap_t* map, const char* key) {
    uint64_t raw_hash = hash_string(key);
    uint64_t bucket_idx = raw_hash % map->capacity;

    const strimap_entry_t* curr = map->buckets[bucket_idx];

    while (curr) {
        if (strcmp(key, curr->key) == 0) {
            return &curr->val;
        }
        curr = curr->next;
    }
    return NULL; // not found
}

void strimap_rehash(strimap_t* map, size_t new_capacity) {
    if (new_capacity < 1) {
        return; // guard
    }
    strimap_entry_t** new_buckets =
        (strimap_entry_t**)calloc(new_capacity, sizeof(strimap_entry_t*));

    for (size_t i = 0; i < map->capacity; i++) {
        strimap_entry_t* curr = map->buckets[i];
        while (curr) {
            strimap_entry_t* next = curr->next;

            uint64_t raw_hash = hash_string(curr->key);
            uint64_t bucket_idx = raw_hash % new_capacity;

            // ins node at head of new bucket
            curr->next = new_buckets[bucket_idx];
            new_buckets[bucket_idx] = curr;

            curr = next;
        }
    }
    free((void*)map->buckets);
    map->buckets = new_buckets;
    map->capacity = new_capacity;
}

bool strimap_contains(const strimap_t* map, const char* key) {
    return strimap_view(map, key) != NULL;
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
