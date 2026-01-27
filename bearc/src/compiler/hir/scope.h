//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_SCOPE_H
#define COMPILER_HIR_SCOPE_H

#include "compiler/hir/indexing.h"
#include "utils/mapu32u32.h"

typedef mapu32u32_t hir_symbol_to_def_map_t;

/// TODO: define helpers for this struct
typedef struct {
    /// module, struct, and variant names
    hir_symbol_to_def_map_t namespaces;
    /// var foo;
    hir_symbol_to_def_map_t variables;
    /// top-level functions
    hir_symbol_to_def_map_t functions;
    /// structs, variants, unions
    hir_symbol_to_def_map_t types;
} hir_scope_table_t;

typedef struct hir_scope {
    hir_scope_id_t opt_parent;
    hir_scope_table_t symbol_table;
} hir_scope_t;

typedef struct hir_scope_anon {
    hir_scope_id_t opt_parent;
    hir_scope_anon_id_t opt_anon_parent;
    hir_scope_table_t symbol_table;
} hir_scope_anon_t;

#endif
