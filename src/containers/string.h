// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#ifndef CONTAINERS_STRING
#define CONTAINERS_STRING
#include "containers/vector.h"

/*
 * string type that owns its own resources. string_destory(string_t) before a string goes out of
 * scope.
 * the underlying vector should not be modified manually
 */
typedef struct {
    vector_t vec; // do not meant to be touched
} string_t;

// ctor
string_t string_create();
// must be a null-terminated string, ideally a string literal
string_t string_from(const char* null_term_string);
string_t string_create_and_reserve(size_t capacity);
// creates a string of a specified length filled with the specified character
string_t string_create_and_fill(size_t len, char c);

// dtor
void string_destroy(string_t* string);

// getters
char* string_get_data(const string_t* string);
size_t string_get_size(const string_t* string);
size_t string_get_capacity(const string_t* string);

// idx
char* string_at_ptr(const string_t* string, size_t idx);
char string_at(const string_t* string, size_t idx);
char* string_start(const string_t* string);

// modifiers

// push a char to the end of the string
bool string_push_char(string_t* string, char c);
// push a string_t onto a string_t
void string_push_string(string_t* dest, const string_t* src);
// push a standard C-string onto a string_t
void string_push_cstring(string_t* string, const char* cstr);
// push a char* with a specified length
void string_push_strn(string_t* string, const char* str, size_t len);
char string_pop_char(string_t* string);
void string_reserve(string_t* string, size_t new_capacity);
bool string_shrink_to_fit(string_t* string);

#endif // !CONTAINERS_STRING
