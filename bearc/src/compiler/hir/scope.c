//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/scope.h"
#include "compiler/hir/indexing.h"
#include "compiler/hir/tables.h"
#include "utils/arena.h"
#include "utils/mapu32u32.h"
#include <assert.h>
#include <stdint.h>
#define HIR_SCOPE_ARENA_DEFAULT_SIZE 0x10000
#define HIR_SCOPE_MAP_DEFAULT_SIZE 0x2000
/// helper for looking into symbol -> def maps
static inline hir_def_id_t hir_symbol_to_def_map_look_up(hir_symbol_to_def_map_t* map,
                                                         hir_symbol_id_t symbol_id) {
    const hir_id_t* res = mapu32u32_cat(map, symbol_id.val);
    hir_def_id_t def = {.val = HIR_ID_NONE};
    if (res == NULL) {
        return def;
    }
    def.val = *res;
    return def;
}

void hir_scope_without_parent(hir_scope_t* scope) {
    hir_scope_init_with_parent(scope, (hir_scope_id_t){.val = HIR_ID_NONE});
}
void hir_scope_init_with_parent(hir_scope_t* scope, hir_scope_id_t parent) {
    arena_t arena = arena_create(HIR_SCOPE_ARENA_DEFAULT_SIZE);
    scope->opt_parent = parent;
    scope->arena = arena;
    scope->functions = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->namespaces = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->variables = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->types = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
}
void hir_scope_destroy(hir_scope_t* scope) { arena_destroy(&scope->arena); }

typedef enum {
    SCOPE_KIND_NAMESPACE,
    SCOPE_KIND_FUNCTION,
    SCOPE_KIND_VARIABLE,
    SCOPE_KIND_TYPE,
} scope_kind_e;

static inline hir_scope_look_up_result_t hir_scope_look_up(hir_tables_t* tables,
                                                           hir_scope_id_t local_scope_id,
                                                           hir_symbol_id_t symbol,
                                                           scope_kind_e kind) {
    if (local_scope_id.val == HIR_ID_NONE) {
        return (hir_scope_look_up_result_t){(hir_def_id_t){.val = HIR_ID_NONE},
                                            HIR_SCOPE_INVALID_SCOPE_SEARCHED};
    }
    // init curr scope local scope
    hir_scope_t* local_scope = vector_at(&tables->scope_vec, local_scope_id.val);
    hir_scope_t* curr_scope = local_scope;
    hir_scope_id_t curr_scope_id = local_scope_id;
    // begin search logic
    hir_def_id_t def = {.val = HIR_ID_NONE};
    hir_scope_look_up_result_status_e status = HIR_SCOPE_LOOK_UP_OKAY;

    // start walking scopes from local thru parents
    while (def.val == HIR_ID_NONE) {
        switch (kind) {
        case SCOPE_KIND_NAMESPACE:
            def = hir_symbol_to_def_map_look_up(&curr_scope->namespaces, symbol);
            break;
        case SCOPE_KIND_FUNCTION:
            def = hir_symbol_to_def_map_look_up(&curr_scope->functions, symbol);
            break;
        case SCOPE_KIND_VARIABLE:
            def = hir_symbol_to_def_map_look_up(&curr_scope->variables, symbol);
            break;
        case SCOPE_KIND_TYPE:
            def = hir_symbol_to_def_map_look_up(&curr_scope->types, symbol);
            break;
        }
        if (def.val) {
            break; // hit, stop now since we allow shadowing
        }
        const hir_scope_id_t parent_scope_id = curr_scope->opt_parent;
        assert((parent_scope_id.val != curr_scope_id.val) && "self-referential scope\n");
        curr_scope_id = parent_scope_id;
        if (curr_scope_id.val == HIR_ID_NONE) {
            break; // no more parents, stop traversing
        }
        curr_scope = vector_at(&tables->scope_vec, curr_scope_id.val);
    }
    if (!def.val) {
        status = HIR_SCOPE_LOOK_UP_NOT_FOUND;
    }
    return (hir_scope_look_up_result_t){.def_id = def, .status = status};
}

hir_scope_look_up_result_t hir_scope_look_up_namespace(hir_tables_t* tables,
                                                       hir_scope_id_t local_scope_id,
                                                       hir_symbol_id_t symbol) {
    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_NAMESPACE);
}
hir_scope_look_up_result_t hir_scope_look_up_variable(hir_tables_t* tables,
                                                      hir_scope_id_t local_scope_id,
                                                      hir_symbol_id_t symbol) {
    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_VARIABLE);
}
hir_scope_look_up_result_t hir_scope_look_up_function(hir_tables_t* tables,
                                                      hir_scope_id_t local_scope_id,
                                                      hir_symbol_id_t symbol) {

    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_FUNCTION);
}
hir_scope_look_up_result_t hir_scope_look_up_types(hir_tables_t* tables,
                                                   hir_scope_id_t local_scope_id,
                                                   hir_symbol_id_t symbol) {

    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_TYPE);
}
