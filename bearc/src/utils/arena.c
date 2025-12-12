// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "utils/arena.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// arena chunk, composes the arena
typedef struct arena_chunk {
    uint8_t* buffer;
    size_t used;
    size_t cap;
    struct arena_chunk* next;
} arena_chunk_t;

// ctor for an arena chunk, private helper
arena_chunk_t* arena_chunk_new(size_t chunk_cap_bytes) {
    arena_chunk_t* chunk = malloc(sizeof(arena_chunk_t));
    chunk->used = 0; // set filled size to zero
    chunk->cap = chunk_cap_bytes;
    chunk->buffer = malloc(chunk_cap_bytes);
    chunk->next = NULL;
    return chunk;
}

// destroys a chain of chunks, private helper
void arena_destroy_chunk_chain(arena_chunk_t* first_chunk) {
    arena_chunk_t* curr = first_chunk;
    arena_chunk_t* next = NULL;
    while (curr) {
        next = curr->next;
        free(curr->buffer); // free chunk's internal buffer
        free(curr);         // free chunk itself
        curr = next;
    }
}

arena_t arena_create(size_t chunk_cap_bytes) {
    arena_t arena;
    arena.chunk_size = chunk_cap_bytes;
    arena.head = arena_chunk_new(chunk_cap_bytes);
    return arena;
}

void arena_destroy(arena_t* arena) { arena_destroy_chunk_chain(arena->head); }

void* arena_alloc(arena_t* arena, size_t req_size_bytes) {
    arena_chunk_t* curr = arena->head;

    // compute aligned start offset for this chunk (using this funky formula), use this instead of
    // curr->used since ptrs need to be 8-byte aligned
    size_t aligned = (curr->used + 7) & ~7;

    // detect overflow or alignment past capacity
    bool invalid_chunk_state = aligned < curr->used || aligned > curr->cap;

    if (invalid_chunk_state || req_size_bytes > curr->cap - aligned) {
        size_t alloc_size = arena->chunk_size;
        if (req_size_bytes > alloc_size) {
            alloc_size = req_size_bytes;
        }

        arena_chunk_t* new_chunk = arena_chunk_new(alloc_size);
        new_chunk->next = curr;
        arena->head = curr = new_chunk;

        // freshly created chunk, used = 0 -> align again
        aligned = (curr->used + 7) & ~7;
    }

    // allocation always begins at the aligned offset
    void* allocation = curr->buffer + aligned;

    // bump the used
    size_t new_used = aligned + req_size_bytes;
    if (new_used < aligned || new_used > curr->cap) {
        // failure/invalidate state
        return NULL;
    }
    curr->used = new_used;

    return allocation;
}

void arena_log_debug_info(arena_t* arena) {
    arena_chunk_t* curr = arena->head;
    arena_chunk_t* next = NULL;
    // stats
    size_t chunk_num = 0;

    while (curr) {
        next = curr->next;
        printf("chunk #%zu\n", chunk_num);
        printf("-> cap : %zu\n", curr->cap);
        printf("-> used: %zu\n", curr->used);
        curr = next;
        chunk_num++;
    }
    printf("total # of chunks: %zu\n", chunk_num);
}
