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

src_buffer_t create_src_buffer_from_file(const char* file_name);
void destroy_src_buffer(src_buffer_t* buffer);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// helpers
bool file_exists(const char* file_name);
int read_file_to_src_buffer(src_buffer_t* buffer);

#endif // !FILE_IO_H
