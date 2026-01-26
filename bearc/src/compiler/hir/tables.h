//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TABLES
#define COMPILER_HIR_TABLES

#include "utils/strimap.h"
#include "utils/vector.h"

/**
 * primary data container for hir structures
 * - *_vec holds hir node data whereas *_id_vec holds simply ids pointing to hir nodes
 * - this model allows for slices with purely ids (no pointers)
 * - all vector tables MUST reserve id/idx 0 to store value 0!
 */
typedef struct {
    strimap_t str_to_symbol_id_map;
    vector_t file_vec;
    vector_t symbol_id_vec;
    vector_t symbol_vec;
    vector_t exec_id_vec;
    vector_t exec_vec;
    vector_t def_id_vec;
    vector_t def_vec;
    vector_t type_id_vec;
    vector_t type_vec;
    vector_t generic_param_id_vec;
    vector_t generic_param_vec;
    vector_t generic_arg_id_vec;
    vector_t generic_arg_vec;
} hir_tables_t;

#endif
