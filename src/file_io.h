#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdbool.h>
#include <stddef.h>

// CHAR BUFFER FROM FILE STRUCT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
    const char* file_name; // non-owning view into filename
    char* data;            // owns, freed by destroy_char_buffer_from_file
    size_t size;
} char_buffer_from_file_t;

char_buffer_from_file_t create_char_buffer_from_file(const char* file_name);
void destroy_char_buffer_from_file(char_buffer_from_file_t* buffer);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// helpers
bool file_exists(const char* file_name);
int read_file_to_char_buffer(char_buffer_from_file_t* buffer);

#endif // !FILE_IO_H
