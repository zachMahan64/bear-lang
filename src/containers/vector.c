#include "containers/vector.h"
#include "log.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// returns a new vector by value, which will eventually need to be destructed by vector_destroy
vector_t vector_create(size_t elem_size) {
    vector_t vec = {.size = 0, .capacity = 2, .elem_size = elem_size};
    vec.data = malloc(elem_size * vec.capacity);
    if (!vec.data) {
        // if malloc fails
        vec.capacity = 0;
    }
    return vec;
}

// returns a new vector with specified capacity
vector_t vector_create_and_reserve(size_t elem_size, size_t capacity) {
    vector_t vec = {.size = 0, .capacity = capacity, .elem_size = elem_size};
    vec.data = malloc(elem_size * vec.capacity);
    if (!vec.data) {
        // if malloc fails
        vec.capacity = 0;
    }
    return vec;
}

// returns a vector with specified number of zero-initialized elements
vector_t vector_create_and_init(size_t elem_size, size_t size) {
    vector_t vec = {.size = 0, .capacity = size, .elem_size = elem_size};
    vec.data = calloc(vec.capacity, elem_size); // zero-init
    if (!vec.data) {
        vec.capacity = 0;
        vec.size = 0;
    }
    return vec;
}

void vector_destroy(vector_t* vector) {
    free(vector->data);
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
}

// get ptr to underlying data
void* vector_get_data(const vector_t* vector) { return vector->data; }
// get size of vector
size_t vector_get_size(const vector_t* vector) { return vector->size; }
// get capacity of vector
size_t vector_get_capacity(const vector_t* vector) { return vector->capacity; }
// get element size of vector
size_t vector_get_elem_size(const vector_t* vector) { return vector->elem_size; }

// idx
void* vector_at(const vector_t* vector, size_t idx) {
    if (idx >= vector->size) {
        LOG_ERR("[ERROR] vector_t: out of range")
        return NULL; // out of bounds
    }
    return (void*)((char*)vector->data + (idx * vector->elem_size));
}

void* vector_start(const vector_t* vector) { return vector->data; }
void* vector_end(const vector_t* vector) {
    return (void*)((char*)vector->data + ((vector->size) * vector->elem_size));
}
void* vector_last(const vector_t* vector) {
    return (void*)((char*)vector->data + ((vector->size - 1) * vector->elem_size));
}

// modifiers
void vector_push_back(vector_t* vector, const void* elem) {
    if (vector->size == vector->capacity) {
        size_t new_capacity = (vector->capacity == 0) ? 1 : vector->capacity * 2;
        void* temp = realloc(vector->data, new_capacity * vector->elem_size);
        if (!temp) {
            LOG_ERR("[ERROR] vector_t: reallocation failed when increasing capacity");
            return; // vector still valid, but push back fails w/ msg
        }
        vector->data = temp;
        vector->capacity = new_capacity;
    }
    memcpy((char*)vector->data + (vector->size * vector->elem_size), elem, vector->elem_size);
    ++vector->size;
}

void vector_remove_back(vector_t* vector) {
    if (vector->size == 0) {
        return; // fail silently cuz empty
    }
    --vector->size;
}
void vector_reserve(vector_t* vector, size_t new_capacity) {
    if (new_capacity <= vector->capacity) {
        return; // we fine
    }
    void* temp = realloc(vector->data, new_capacity * vector->elem_size);
    if (!temp) {
        LOG_ERR("[ERROR] vector_t: reallocation failed when reserving");
        return; // vector still valid, but fails w/ msg
    }
    vector->data = temp;
    vector->capacity = new_capacity;
}

void vector_shrink_to_fit(vector_t* vector) {
    if (vector->size == vector->capacity) {
        return;
    }
    void* temp = realloc(vector->data, vector->size * vector->elem_size);
    if (!temp) {
        LOG_ERR("[ERROR] vector_t: reallocation failed when shrinking to fit");
        return;
    }
    vector->data = temp;
    vector->capacity = vector->size;
}
