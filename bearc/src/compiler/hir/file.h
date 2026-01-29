//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_FILE_H
#define COMPILER_HIR_FILE_H
#include "compiler/hir/indexing.h"
typedef enum {
    FILE_LOAD_STATE_UNVISITING = 0,
    FILE_LOAD_STATE_IN_PROGRESS,
    FILE_LOAD_STATE_DONE
} file_load_state_e;

typedef struct {
    hir_symbol_id_t path;
    file_load_state_e load_state;
} hir_file_t;

typedef struct {
    hir_file_id_idx_t start;
    hir_size_t len;
} hir_file_dependencies_slice_t;

#endif
