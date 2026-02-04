//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_DEF_H
#define COMPILER_HIR_DEF_H

#include "compiler/hir/indexing.h"
#include "compiler/hir/span.h"

// ------ struct impls -------
// TODO: finish impl structures

typedef enum {
    HIR_DEF_MODULE,
} hir_def_tag_e;

typedef struct {
    hir_scope_id_t scope;
} hir_def_module_t;

// ^^^^^^ struct impls ^^^^^^^^

/// main exec union
typedef union {
    hir_def_module_t module;
} hir_def_u;

/// main exec structure, corresponds to an hir_exec_id_t
typedef struct {
    hir_def_u def;
    span_t span;
    hir_def_tag_e tag;
    /// value of HIR_ID_NONE (0) indicates no parent
    hir_def_id_t def_id;
    bool resolved;
    bool top_level_visited;
    bool pub;
} hir_def_t;

#endif
