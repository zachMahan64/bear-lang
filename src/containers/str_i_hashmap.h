#ifndef CONTAINERS_STR_I_HASHMAP_H
#define CONTAINERS_STR_I_HASHMAP_H

#include <stddef.h>

struct str_i_hashmap_entry;

typedef struct {
    char* key;
    int val;
    struct str_i_hashmap_entry* next;
} str_i_hashmap_entry;

typedef struct {
    str_i_hashmap_entry** buckets;
    size_t size;
} str_i_hashmap;

#endif // ! CONTAINERS_STR_I_HASHMAP_H
