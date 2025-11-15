// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef CONTAINERS_ARENA_H
#define CONTAINERS_ARENA_H

#include <stddef.h>
#include <stdint.h>

struct arena_chunk;

// main arena container
typedef struct arena {
    struct arena_chunk* head;
    size_t chunk_size;
} arena_t;

// arena ctor (init) from a specified standard chunk size.
arena_t arena_create(size_t chunk_cap_bytes);
// arena dtor (clean up resources)
void arena_destroy(arena_t* arena); // TODO define this
// get an allocation from the arena of a specified size
void* arena_alloc(arena_t* arena, size_t req_size_bytes);

// testing
void arena_log_debug_info(arena_t* arena);

#endif // !CONTAINERS_ARENA_H
