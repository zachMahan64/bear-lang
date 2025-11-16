#ifndef CONTAINERS_STRING_VIEW
#define CONTAINERS_STRING_VIEW
#include <stddef.h>

typedef struct {
    const char* start;
    size_t len;
} string_view_t;

#endif // !CONTAINERS_STRING_VIEW
