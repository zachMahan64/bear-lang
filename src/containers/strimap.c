#include "strimap.h"
#include <stdint.h>
#include <stdlib.h>

strimap_t strimap_create(size_t capacity) {
    strimap_t map;
    map.capacity = capacity;
    map.buckets = (strimap_entry_t**)calloc(capacity,
                                            sizeof(strimap_entry_t*)); // Initialize to NULL
    return map;
}
void strimap_destroy(strimap_t* map) {
    if (!map || !map->buckets) {
        return;
    }
    for (int i = 0; i < map->capacity; i++) {
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

uint64_t hash_string(const char* str) {
    unsigned long hash = 5381;
    int curr_ch;

    while ((curr_ch = (unsigned char)*str++)) { // iter
        // hash = hash * 33 + c (DJB2 algorithm)
        hash = ((hash << 5) + hash) + curr_ch;
    }

    return hash;
}

void strimap_insert(strimap_t* map, char* key, int val) {}

void strimap_rehash(strimap_t* map, size_t new_capacity) {}

// TODO finish impl
