#ifndef VECTOR_H
#define VECTOR_H
#include <stddef.h>

// generic vector,
typedef struct {
    void* data;
    size_t size;
    size_t capacity;
    const size_t elem_size;
} vector_t;

// ctor
vector_t vector_create(size_t elem_size);
vector_t vector_create_and_reserve(size_t elem_size, size_t capacity);
vector_t vector_create_and_init(size_t elem_size, size_t size);
// dtor
void vector_destroy(vector_t* vector);

// getters
void* vector_get_data(const vector_t* vector);
size_t vector_get_size(const vector_t* vector);
size_t vector_get_capacity(const vector_t* vector);
size_t vector_get_elem_size(const vector_t* vector);

// idx
void* vector_at(const vector_t* vector, size_t idx);
void* vector_start(const vector_t* vector);
void* vector_end(const vector_t* vector);
void* vector_last(const vector_t* vector);
// modifiers
void vector_push_back(vector_t* vector, const void* elem);
void vector_remove_back(vector_t* vector);
void vector_reserve(vector_t* vector, size_t new_capacity);
void vector_shrink_to_fit(vector_t* vector);

#endif // !VECTOR_H
