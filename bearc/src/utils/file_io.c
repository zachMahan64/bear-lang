//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "utils/file_io.h"
#include "utils/ansi_codes.h"
#include <stdio.h>
#include <stdlib.h>

// check that a file of a specified name exists
bool file_exists(const char* file_name) {
    FILE* file = fopen(file_name, "r");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

// helper
int read_file_to_src_buffer(src_buffer_t* buffer) {
    FILE* file = fopen(buffer->file_name, "rb");
    if (!file) {
        return -1;
    }

    // find file size
    if (fseek(file, 0, SEEK_END) != 0) {
        fclose(file);
        return -1;
    }
    long size = ftell(file);
    if (size < 0) {
        fclose(file);
        return -1;
    }
    if (fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return -1;
    }

    // allocate buffer (+1 for null terminator in case treating as string)
    buffer->data = malloc((size_t)size + 1);
    if (!buffer->data) {
        fclose(file);
        return -1;
    }

    // read file into buffer
    size_t read_size = fread(buffer->data, 1, (size_t)size, file);
    fclose(file);

    if (read_size != (size_t)size) {
        free(buffer->data);
        return -1;
    }

    buffer->data[size] = '\0'; // safe even if binary, optional
    buffer->size = (size_t)size;
    return 0;
}

// returns an src_buffer_t by value, which will need to be destructed by src_buffer_destroy
src_buffer_t src_buffer_from_file_create(const char* file_name) {
    src_buffer_t buffer;
    buffer.file_name = file_name;
    if (read_file_to_src_buffer(&buffer) < 0) {
        printf("%serror%s: could not read file: %s", ansi_bold_red(), ansi_reset(), file_name);
    }
    return buffer;
}

// destructs an src_buffer_t that was created by src_buffer_from_file_create
void src_buffer_destroy(src_buffer_t* buffer) { free(buffer->data); }

// gets ptr to data
const char* src_buffer_get(src_buffer_t* buffer) { return buffer->data; }
