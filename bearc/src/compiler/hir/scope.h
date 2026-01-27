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
#include "utils/vector.h"

typedef mapu32u32_t hir_symbol_to_def_map_t;

/// TODO: define acessors, mutator, ctors, and dtors for these structs

typedef struct hir_scope {
    hir_scope_id_t opt_parent;
    /// module, struct, and variant names
    hir_symbol_to_def_map_t namespaces;
    /// var foo;
    hir_symbol_to_def_map_t variables;
    /// top-level functions
    hir_symbol_to_def_map_t functions;
    /// structs, variants, unions, deftypes
    hir_symbol_to_def_map_t types;

} hir_scope_t;

typedef struct hir_scope_anon {
    hir_scope_id_t opt_parent;
    hir_scope_anon_id_t opt_anon_parent;
    /// var foo;
    hir_symbol_to_def_map_t variables;
    /// structs, variants, unions, deftypes
    hir_symbol_to_def_map_t types;
    /// modules, struct, and variant defs brought in
    vector_t used_defs_hir_def_id;
    bool has_used_defs; // so that we can lazily init the used_defs_hir_def_id vector
} hir_scope_anon_t;

#endif
