#include "containers/vector.h"
#include <stdlib.h>

// ctors/dtors
vector_t vector_create(size_t elem_size) {
    vector_t vec;
    vec.size = 0;
    vec.capacity = 2;
    vec.elem_size = elem_size;
    vec.data = malloc(elem_size * vec.capacity);
    if (!vec.data) {
        // if malloc fails
        vec.capacity = 0;
    }
    return vec;
}

vector_t vector_create_and_reserve(size_t elem_size, size_t capacity) {
    vector_t vec;
    vec.size = 0;
    vec.capacity = capacity;
    vec.elem_size = elem_size;
    vec.data = malloc(elem_size * vec.capacity);
    if (!vec.data) {
        // if malloc fails
        vec.capacity = 0;
    }
    return vec;
}

void vector_destroy(vector_t* vector) { free(vector->data); }

// getters
void* vector_get_data(const vector_t* vector) { return vector->data; }
size_t vector_get_size(const vector_t* vector) { return vector->size; }
size_t vector_get_capacity(const vector_t* vector) { return vector->capacity; }
size_t vector_get_elem_size(const vector_t* vector) { return vector->elem_size; }

// idx
void* vector_at(const vector_t* vector, size_t idx) {
    if (idx >= vector->size) {
        return NULL; // out of bounds
    }
    return (void*)((char*)vector->data + (idx * vector->elem_size));
}

void* vector_start(const vector_t* vector) { return vector->data; }
void* vector_end(const vector_t* vector) {
    return (void*)((char*)vector->data + ((vector->size - 1) * vector->elem_size));
}

// modifiers
void vector_push_back(vector_t* vector, const void* elem) {
    ++vector->size;
    if (vector->size > vector->capacity) {
        void* temp = malloc(2 * vector->capacity);
        // TOOD finish
    }
}
void vector_remove_back(vector_t* vector);
void vector_reserve(vector_t* vector, size_t new_capacity);
