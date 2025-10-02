#include "strimap.h"
#include <stdlib.h>

strimap_t strimap_create(size_t size) {
    strimap_t map;
    map.size = size;
    map.buckets = (strimap_entry_t**)calloc(size, sizeof(strimap_entry_t*)); // Initialize to NULL
    return map;
}
void strimap_destroy(strimap_t* map) {
    for (int i = 0; i < map->size; i++) {
        strimap_entry_t* curr = map->buckets[i];
        strimap_entry_t* next = curr->next;
        while (next) {
            free(curr);
            curr = next;
        }
    }
}

// TODO finish impl
