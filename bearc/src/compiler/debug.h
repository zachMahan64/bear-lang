//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_DEBUG_H
#define COMPILER_DEBUG_H

#include "utils/file_io.h"
#include "utils/vector.h"

// print out contents of src in debug builds
void print_out_src_buffer(src_buffer_t* src_buffer);
// print out lexed token tablw in debug builds
void print_out_tkn_table(vector_t* vec);

#endif
