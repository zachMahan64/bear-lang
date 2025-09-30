#include "strimap.h"
#include <stdlib.h>

strimap_t strimap_create(size_t size) {
    strimap_t map;
    map.size = size;
    map.buckets = (strimap_entry_t**)calloc(size, sizeof(strimap_entry_t*)); // Initialize to NULL
    return map;
}

// TODO finish impl
