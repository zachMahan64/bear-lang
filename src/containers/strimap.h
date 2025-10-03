#ifndef CONTAINERS_STRIMAP_H
#define CONTAINERS_STRIMAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define STRIMAP_MINIMUM_CAPACITY 8
#define STRIMAP_LOAD_FACTOR .75

typedef struct strimap_entry_t {
    char* key;
    int val;
    struct strimap_entry_t* next;
} strimap_entry_t;

typedef struct {
    strimap_entry_t** buckets;
    size_t capacity;
    size_t size;
} strimap_t;

// strimap ctor/dtor
strimap_t strimap_create(size_t capacity);
void strimap_destroy(strimap_t* map);

// mutators
void strimap_insert(strimap_t* map, const char* key, int val);
void strimap_remove(strimap_t* map, const char* key);
void strimap_rehash(strimap_t* map, size_t new_capacity);
int* strimap_at(strimap_t* map, const char* key);

// viewers
const int* strimap_view(const strimap_t* map, const char* key);
bool strimap_contains(const strimap_t* map, const char* key);

// TODO impl iterator
typedef struct {
    strimap_t* map;
    size_t bucket_idx;
    strimap_entry_t* curr;
} strimap_iter_t;
strimap_iter_t strimap_create_iter(strimap_t* map);
void strimap_iter_next(strimap_iter_t* iter, char* key, int* val);

// helper
uint64_t hash_string(const char* str);
#endif // ! CONTAINERS_STRIMAP_H
