#include "interpreter/file_io.h"
#include <stdio.h>
bool file_exists(const char* file_name) {
    FILE* file = fopen(file_name, "r");
    if (file) {
        printf("[DEBUG] File '%s' exists\n", file_name);
        fclose(file);
        return true;
    }
    printf("[ERROR] File '%s' does not exist\n", file_name);
    return false;
}

#include <stdio.h>
#include <stdlib.h>

char* file_read_to_buffer(const char* filename, size_t* out_size) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        return NULL;
    }

    // find file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }

    // allocate buffer (+1 for null terminator in case treating as string)
    char* buffer = malloc((size_t)size + 1);
    if (!buffer) {
        fclose(file);
        return NULL;
    }

    // read file into buffer
    size_t read_size = fread(buffer, 1, (size_t)size, file);
    fclose(file);

    if (read_size != (size_t)size) {
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0'; // safe even if binary, optional
    if (out_size) {
        *out_size = (size_t)size;
    }
    return buffer;
}
