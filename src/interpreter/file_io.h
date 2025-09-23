#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdbool.h>
#include <stddef.h>

bool file_exists(const char* file_name);
char* file_read_to_buffer(const char* filename, size_t* out_size);

#endif // !FILE_IO_H
