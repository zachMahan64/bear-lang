#include "compile.h"
#include "file_io.h"
#include "log.h"
#include "stddef.h"

int compile_file(const char* file_name) {
    int error_code = 0; // return error code if hit error
    src_buffer_t buffer = create_src_buffer_from_file(file_name);
    /*
     * TODO compile logic
     * output to a bytecode file eventually
     */
    printf("File data:\n~~~~~\n%s\n~~~~~~\n", buffer.data);

    /* TODO, at first, just output one unified bytecode file
     * with original_file_name.bvm
     */

    destroy_src_buffer(&buffer);
    return error_code;
}
