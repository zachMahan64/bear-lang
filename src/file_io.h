#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdbool.h>
#include <stddef.h>

// CHAR BUFFER FROM FILE STRUCT ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct {
    const char* file_name; // non-owning view into filename
    char* data;            // owns, freed by destroy_char_buffer_from_file
    size_t size;           // size of char buffer, in bytes/chars
} src_buffer_t;

src_buffer_t src_buffer_from_file_create(const char* file_name);
void src_buffer_destroy(src_buffer_t* buffer);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

bool file_exists(const char* file_name);

#endif // !FILE_IO_H
