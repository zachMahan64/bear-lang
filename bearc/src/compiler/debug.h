#ifndef COMPILER_DEBUG_H
#define COMPILER_DEBUG_H

#include "utils/file_io.h"
#include "utils/vector.h"

// print out contents of src in debug builds
void print_out_src_buffer(src_buffer_t* src_buffer);
// print out lexed token tablw in debug builds
void print_out_tkn_table(vector_t* vec);

#endif
