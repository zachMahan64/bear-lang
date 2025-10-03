#include "compile.h"
#include "file_io.h"
#include "log.h"
#include "stddef.h"

int compile_file(const char* file_name) {
    int error_code = 0; // return error code if hit error
    src_buffer_t buffer = src_buffer_from_file_create(file_name);
    /*
     * TODO compile logic
     * output to a bytecode file eventually
     */
    printf("File data:\n~~~~~\n%s\n~~~~~~\n", buffer.data);

    /* TODO, at first, just output one unified bytecode file
     * with original_file_name.bvm
     */

    src_buffer_destroy(&buffer);
    return error_code;
}
