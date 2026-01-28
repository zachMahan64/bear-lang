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
#include "utils/arena.h"
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
    /// for shared flat map storage, not owned
    arena_t arena;
} hir_scope_t;

void hir_scope_without_parent(hir_scope_t* scope, arena_t arena);
void hir_scope_init_with_parent(hir_scope_t* scope, hir_scope_id_t parent, arena_t arena);

hir_scope_look_up_result_t hir_scope_look_up_namespace(hir_tables_t* tables,
                                                       hir_scope_id_t local_scope,
                                                       hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_look_up_variable(hir_tables_t* tables,
                                                      hir_scope_id_t local_scope,
                                                      hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_look_up_function(hir_tables_t* tables,
                                                      hir_scope_id_t local_scope,
                                                      hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_look_up_type(hir_tables_t* tables, hir_scope_id_t local_scope,
                                                  hir_symbol_id_t symbol);

void hir_scope_insert_namespace(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def);
void hir_scope_insert_function(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def);
void hir_scope_insert_variable(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def);
void hir_scope_insert_type(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def);

/**
 * maps hir_symbol_id_t -> hir_def_id
 * models anonymous blocks, such as function bodies or ctrl flow blocks
 */
typedef struct hir_scope_anon {
    hir_scope_id_t opt_named_parent;
    hir_scope_anon_id_t opt_anon_parent;
    /// structs, variants, unions, deftypes
    hir_symbol_to_def_map_t types;
    /// var foo;
    hir_symbol_to_def_map_t variables;
    /// top-level functions
    hir_symbol_to_def_map_t functions;
    /// module, struct, and variant names
    hir_symbol_to_def_map_t namespaces;
    /// modules, struct, and variant defs brought in
    vector_t used_hir_def_ids;
    /// for shared flat map storage
    arena_t arena;
    bool is_top_level;
    bool has_used_defs; // so that we can lazily init the used_defs_hir_def_id vector
} hir_scope_anon_t;

void hir_scope_anon_top_level_init(hir_scope_anon_t* scope, arena_t arena);
void hir_scope_anon_init_with_named_parent(hir_scope_anon_t* scope, hir_scope_id_t named_parent,
                                           arena_t arena);
void hir_scope_anon_init_with_anon_parent(hir_scope_anon_t* scope, hir_scope_anon_id_t anon_parent,
                                          arena_t arena);
void hir_scope_anon_destroy(hir_scope_anon_t* scope);

hir_scope_look_up_result_t hir_scope_anon_look_up_namespace(hir_tables_t* tables,
                                                            hir_scope_anon_id_t local_scope,
                                                            hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_anon_look_up_variable(hir_tables_t* tables,
                                                           hir_scope_anon_id_t local_scope,
                                                           hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_anon_look_up_function(hir_tables_t* tables,
                                                           hir_scope_anon_id_t local_scope,
                                                           hir_symbol_id_t symbol);
hir_scope_look_up_result_t hir_scope_anon_look_up_type(hir_tables_t* tables,
                                                       hir_scope_anon_id_t local_scope,
                                                       hir_symbol_id_t symbol);
void hir_scope_anon_insert_namespace(hir_scope_anon_t* scope, hir_symbol_id_t symbol,
                                     hir_def_id_t def);
void hir_scope_anon_insert_function(hir_scope_anon_t* scope, hir_symbol_id_t symbol,
                                    hir_def_id_t def);
void hir_scope_anon_insert_variable(hir_scope_anon_t* scope, hir_symbol_id_t symbol,
                                    hir_def_id_t def);
void hir_scope_anon_insert_type(hir_scope_anon_t* scope, hir_symbol_id_t symbol, hir_def_id_t def);
/// adds a used module to non-top-level anonymous scope
void hir_scope_anon_add_used_module(hir_scope_anon_t* scope, hir_def_id_t def_id);

#endif
