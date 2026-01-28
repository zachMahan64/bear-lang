//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_EXEC_H
#define COMPILER_HIR_EXEC_H

#include "compiler/hir/indexing.h"
#include "compiler/hir/span.h"

// ------ struct impls -------
// TODO: finish impl structures

typedef enum {
    HIR_EXEC_STMT_BLOCK,
} hir_exec_tag_e;

typedef struct {
    hir_scope_anon_id_t scope;
    hir_exec_id_slice_t execs;
} hir_exec_stmt_block_t;

// ^^^^^^ struct impls ^^^^^^^^

/// main exec union
typedef union {
    hir_exec_stmt_block_t block;
} hir_exec_u;

/// main exec structure, corresponds to an hir_exec_id_t
typedef struct {
    hir_exec_u exec;
    span_t span;
    hir_exec_tag_e tag;
    bool compt;
} hir_exec_t;

#endif
