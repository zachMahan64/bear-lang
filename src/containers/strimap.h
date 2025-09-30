#ifndef CONTAINERS_STRIMAP_H
#define CONTAINERS_STRIMAP_H

#include <stddef.h>

struct strimap_entry;

typedef struct {
    char* key;
    int val;
    struct strimap* next;
} strimap_entry_t;

typedef struct {
    strimap_entry_t** buckets;
    size_t size;
} strimap_t;

strimap_t strimap_create(size_t size);
void strimap_destroy(strimap_t* map);
void strimap_insert(strimap_t* map, char* key, int val);
int strimap_at(strimap_t* map, char* key);

#endif // ! CONTAINERS_STRIMAP_H
