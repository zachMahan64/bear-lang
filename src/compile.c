#include "compile.h"
#include "file_io.h"
#include "log.h"
#include "stddef.h"

int compile_file(const char* file_name) {
    int error_code = 0; // return error code if hit error
    char_buffer_from_file_t buffer = create_char_buffer_from_file(file_name);
    /*
     * TODO compile logic
     * output to a bytecode file eventually
     */
    printf("File data: delim[\n%s\n]delim\n", buffer.data);

    destroy_char_buffer_from_file(&buffer);
    return error_code;
}
