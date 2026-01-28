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
#include "compiler/hir/tables.h"
#include "utils/mapu32u32.h"
#include "utils/vector.h"

typedef mapu32u32_t hir_symbol_to_def_map_t;

typedef enum {
    HIR_SCOPE_LOOK_UP_OKAY = 0,
    HIR_SCOPE_INVALID_SCOPE_SEARCHED,
    HIR_SCOPE_LOOK_UP_COLLISION,
    HIR_SCOPE_LOOK_UP_NOT_FOUND,
} hir_scope_look_up_result_status_e;

typedef struct {
    hir_def_id_t def_id;
    hir_scope_look_up_result_status_e status;
} hir_scope_look_up_result_t;

/// TODO: define acessors, mutator, ctors, and dtors for these structs

/**
 * maps hir_symbol_id_t -> hir_def_id
 * models named blocks/namespaces, such as function bodies or ctrl flow blocks
 */
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
    /// for shared flat map storage
    arena_t arena;
} hir_scope_t;

void hir_scope_without_parent(hir_scope_t* scope);
void hir_scope_init_with_parent(hir_scope_t* scope, hir_scope_id_t parent);
void hir_scope_destroy(hir_scope_t* scope);

hir_scope_look_up_result_t hir_scope_look_up_namespace(hir_tables_t* tables,
                                                       hir_scope_id_t local_scope,
                                                       hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_look_up_variable(hir_tables_t* tables,
                                                      hir_scope_id_t local_scope,
                                                      hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_look_up_function(hir_tables_t* tables,
                                                      hir_scope_id_t local_scope,
                                                      hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_look_up_types(hir_tables_t* tables, hir_scope_id_t local_scope,
                                                   hir_symbol_id_t symbol);

/**
 * maps hir_symbol_id_t -> hir_def_id
 * models anonymous blocks, such as function bodies or ctrl flow blocks
 */
typedef struct hir_scope_anon {
    hir_scope_id_t opt_parent;
    hir_scope_anon_id_t opt_anon_parent;
    /// var foo;
    hir_symbol_to_def_map_t variables;
    /// structs, variants, unions, deftypes
    hir_symbol_to_def_map_t types;
    /// modules, struct, and variant defs brought in
    vector_t used_defs_hir_def_id;
    /// for shared flat map storage
    arena_t arena;
    bool has_used_defs; // so that we can lazily init the used_defs_hir_def_id vector
} hir_scope_anon_t;

hir_scope_look_up_result_t hir_anon_scope_look_up_namespace(hir_scope_id_t local_scope,
                                                            hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_anon_scope_look_up_variable(hir_scope_id_t local_scope,
                                                           hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_anon_scope_look_up_function(hir_scope_id_t local_scope,
                                                           hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_anon_scope_look_up_types(hir_scope_id_t local_scope,
                                                        hir_symbol_id_t symbol);

void hir_scope_anon_init(hir_scope_anon_t* scope, hir_scope_id_t named_parent,
                         hir_scope_anon_t anon_parent);
void hir_scope_anon_destroy(hir_scope_anon_t* scope);

#endif
