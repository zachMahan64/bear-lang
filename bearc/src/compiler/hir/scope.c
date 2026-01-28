//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/scope.h"
#include "compiler/hir/def.h"
#include "compiler/hir/indexing.h"
#include "compiler/hir/tables.h"
#include "utils/arena.h"
#include "utils/mapu32u32.h"
#include "utils/string_view.h"
#include "utils/vector.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
// this may need to be tuned for a balance between cache locality and limited rehashing
#define HIR_SCOPE_MAP_DEFAULT_SIZE 0x800
#define HIR_SCOPE_TOP_LEVEL_SCALE_FACTOR 4
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

void hir_scope_without_parent(hir_scope_t* scope, arena_t arena) {
    hir_scope_init_with_parent(scope, (hir_scope_id_t){.val = HIR_ID_NONE}, arena);
}
void hir_scope_init_with_parent(hir_scope_t* scope, hir_scope_id_t parent, arena_t arena) {
    scope->opt_parent = parent;
    scope->arena = arena;
    scope->functions = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->namespaces = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->variables = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->types = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
}
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
        curr_scope = vector_at(&tables->scope_vec, curr_scope_id.val);
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
hir_scope_look_up_result_t hir_scope_look_up_type(hir_tables_t* tables,
                                                  hir_scope_id_t local_scope_id,
                                                  hir_symbol_id_t symbol) {

    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_TYPE);
}

// ------------------ ANON SCOPE IMPL ----------------------

void hir_scope_anon_top_level_init(hir_scope_anon_t* scope, arena_t arena) {
    scope->opt_named_parent.val = HIR_ID_NONE;
    scope->opt_anon_parent.val = HIR_ID_NONE;
    scope->arena = arena;
    scope->variables = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HIR_SCOPE_TOP_LEVEL_SCALE_FACTOR, arena);
    scope->types = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HIR_SCOPE_TOP_LEVEL_SCALE_FACTOR, arena);
    // top-level WILL need this
    scope->functions = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HIR_SCOPE_TOP_LEVEL_SCALE_FACTOR, arena);

    scope->namespaces = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HIR_SCOPE_TOP_LEVEL_SCALE_FACTOR, arena);
    scope->is_top_level = true;
    scope->has_used_defs = false;
}

static inline void hir_scope_anon_init_impl(hir_scope_anon_t* scope, hir_scope_id_t named_parent,
                                            hir_scope_anon_id_t anon_parent, arena_t arena) {
    scope->opt_named_parent = named_parent;
    scope->opt_anon_parent = anon_parent;
    scope->arena = arena;
    scope->variables = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->types = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    // unused, so zero-init for more safety
    scope->functions = (mapu32u32_t){.arena = NULL, .size = 0, .buckets = 0, .capacity = 0};
    scope->namespaces = scope->functions;
    scope->is_top_level = false;
    scope->has_used_defs = false;
}
void hir_scope_anon_init_with_named_parent(hir_scope_anon_t* scope, hir_scope_id_t named_parent,
                                           arena_t arena) {
    hir_scope_anon_init_impl(scope, named_parent, (hir_scope_anon_id_t){.val = HIR_ID_NONE}, arena);
}
void hir_scope_anon_init_with_anon_parent(hir_scope_anon_t* scope, hir_scope_anon_id_t anon_parent,
                                          arena_t arena) {
    hir_scope_anon_init_impl(scope, (hir_scope_id_t){.val = HIR_ID_NONE}, anon_parent, arena);
}
void hir_scope_anon_destroy(hir_scope_anon_t* scope) {
    if (scope->has_used_defs) {
        vector_destroy(&scope->used_hir_def_ids);
    }
}

static inline hir_scope_look_up_result_t hir_scope_anon_look_up(hir_tables_t* tables,
                                                                hir_scope_anon_id_t local_scope_id,
                                                                hir_symbol_id_t symbol,
                                                                scope_kind_e kind) {
    if (local_scope_id.val == HIR_ID_NONE) {
        return (hir_scope_look_up_result_t){(hir_def_id_t){.val = HIR_ID_NONE},
                                            HIR_SCOPE_INVALID_SCOPE_SEARCHED};
    }
    // init curr scope local scope
    hir_scope_anon_t* local_scope_anon = vector_at(&tables->scope_anon_vec, local_scope_id.val);
    hir_scope_anon_t* curr_scope_anon = local_scope_anon;
    hir_scope_t* curr_scope_named = NULL;
    hir_scope_anon_id_t curr_scope_anon_id = local_scope_id;
    hir_scope_id_t curr_scope_named_id = {.val = HIR_ID_NONE};
    // begin search logic
    hir_def_id_t result_def = {.val = HIR_ID_NONE};
    hir_scope_look_up_result_status_e status = HIR_SCOPE_LOOK_UP_OKAY;

    // search used modules first to allow local shadowing!
    hir_def_id_t def_from_used_modules = {.val = HIR_ID_NONE};
    bool collision = false;
    if (local_scope_anon->has_used_defs) {

        assert(!local_scope_anon->is_top_level);

        vector_t* vec = &local_scope_anon->used_hir_def_ids;

        for (hir_size_t i = 0; i < vec->size; i++) {

            hir_def_id_t def_id = *(hir_def_id_t*)vector_at(vec, i);

            // TODO replace with encapsulated tables logic once the tables impl is written
            hir_def_t* used_def = (hir_def_t*)vector_at(&tables->def_vec, def_id.val);
            assert(used_def->tag == HIR_DEF_MODULE);
            hir_scope_look_up_result_t used_res
                = hir_scope_look_up(tables, used_def->def.module.scope, symbol, kind);

            if (used_res.status == HIR_SCOPE_LOOK_UP_OKAY) {
                if (def_from_used_modules.val != HIR_ID_NONE) {
                    collision = true;
                }
                def_from_used_modules = used_res.def_id;
            }
        }
    }

    // start walking scopes from local thru parents
    // exactly one of (curr_scope_anon_id.val, curr_scope_named_id.val) must be nonzero at every
    // step
    while (result_def.val == HIR_ID_NONE) {
        assert(!(curr_scope_anon_id.val && curr_scope_named_id.val));

        if (curr_scope_anon_id.val) {
            curr_scope_anon = vector_at(&tables->scope_anon_vec, curr_scope_anon_id.val);
            switch (kind) {
            case SCOPE_KIND_NAMESPACE:
                // namespace decls only allowed at top-level anon
                assert(curr_scope_anon->is_top_level);
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_anon->namespaces, symbol);
                break;
            case SCOPE_KIND_FUNCTION:
                // functions decls only allowed at top-level anon
                assert(curr_scope_anon->is_top_level);
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_anon->functions, symbol);
                break;
            case SCOPE_KIND_VARIABLE:
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_anon->variables, symbol);
                break;
            case SCOPE_KIND_TYPE:
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_anon->types, symbol);
                break;
            }
            if (result_def.val) {
                break; // hit, stop now since we allow shadowing
            }
            const hir_scope_anon_id_t parent_scope_anon_id = curr_scope_anon->opt_anon_parent;
            assert((parent_scope_anon_id.val != curr_scope_anon_id.val)
                   && "self-referential scope\n");
            curr_scope_anon_id = parent_scope_anon_id;

            const hir_scope_id_t parent_scope_named_id = curr_scope_anon->opt_named_parent;
            assert((parent_scope_named_id.val != curr_scope_named_id.val)
                   && "self-referential scope\n");
            curr_scope_named_id = parent_scope_named_id;
        } else {
            assert(curr_scope_named_id.val);
            curr_scope_named = vector_at(&tables->scope_vec, curr_scope_named_id.val);
            switch (kind) {
            case SCOPE_KIND_NAMESPACE:
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_named->namespaces, symbol);
                break;
            case SCOPE_KIND_FUNCTION:
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_named->functions, symbol);
                break;
            case SCOPE_KIND_VARIABLE:
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_named->variables, symbol);
                break;
            case SCOPE_KIND_TYPE:
                result_def = hir_symbol_to_def_map_look_up(&curr_scope_named->types, symbol);
                break;
            }
            if (result_def.val) {
                break; // hit, stop now since we allow shadowing
            }

            const hir_scope_id_t parent_scope_named_id = curr_scope_named->opt_parent;
            assert((parent_scope_named_id.val != curr_scope_named_id.val)
                   && "self-referential scope\n");
            curr_scope_named_id = parent_scope_named_id;
            curr_scope_anon_id.val = HIR_ID_NONE;
        }
        if (curr_scope_anon_id.val == HIR_ID_NONE && curr_scope_named_id.val == HIR_ID_NONE) {
            break; // no more parents, stop traversing
        }
    }
    if (result_def.val == HIR_ID_NONE) {
        // didn't find a local symbol -> now check the imports
        if (collision) {
            status = HIR_SCOPE_LOOK_UP_COLLISION;
        } else if (def_from_used_modules.val != HIR_ID_NONE) {
            result_def = def_from_used_modules;
            status = HIR_SCOPE_LOOK_UP_OKAY;
        } else {
            status = HIR_SCOPE_LOOK_UP_NOT_FOUND;
        }
    }
    return (hir_scope_look_up_result_t){.def_id = result_def, .status = status};
}

hir_scope_look_up_result_t hir_scope_anon_look_up_namespace(hir_tables_t* tables,
                                                            hir_scope_anon_id_t local_scope,
                                                            hir_symbol_id_t symbol) {
    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_NAMESPACE);
}
hir_scope_look_up_result_t hir_scope_anon_look_up_variable(hir_tables_t* tables,
                                                           hir_scope_anon_id_t local_scope,
                                                           hir_symbol_id_t symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_VARIABLE);
}
hir_scope_look_up_result_t hir_scope_anon_look_up_function(hir_tables_t* tables,
                                                           hir_scope_anon_id_t local_scope,
                                                           hir_symbol_id_t symbol) {
    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_FUNCTION);
}
hir_scope_look_up_result_t hir_scope_anon_look_up_type(hir_tables_t* tables,
                                                       hir_scope_anon_id_t local_scope,
                                                       hir_symbol_id_t symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_TYPE);
}

// ------------------ insert helpers ----------------------

/// insert symbol -> def into a named hir_scope
static inline void hir_scope_insert(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def,
                                    scope_kind_e kind) {
    switch (kind) {
    case SCOPE_KIND_NAMESPACE:
        mapu32u32_insert(&scope->namespaces, symbol.val, def.val);
        break;
    case SCOPE_KIND_FUNCTION:
        mapu32u32_insert(&scope->functions, symbol.val, def.val);
        break;
    case SCOPE_KIND_VARIABLE:
        mapu32u32_insert(&scope->variables, symbol.val, def.val);
        break;
    case SCOPE_KIND_TYPE:
        mapu32u32_insert(&scope->types, symbol.val, def.val);
        break;
    }
}

/// insert symbol -> def into an anonymous hir_scope_anon
static inline void hir_scope_anon_insert(hir_scope_anon_t* scope, hir_symbol_id_t symbol,
                                         hir_def_id_t def, scope_kind_e kind) {
    switch (kind) {
    case SCOPE_KIND_NAMESPACE:
        assert(scope->is_top_level); // only top-level anon scopes hold namespaces
        mapu32u32_insert(&scope->namespaces, symbol.val, def.val);
        break;
    case SCOPE_KIND_FUNCTION:
        assert(scope->is_top_level); // only top-level anon scopes hold functions
        mapu32u32_insert(&scope->functions, symbol.val, def.val);
        break;
    case SCOPE_KIND_VARIABLE:
        mapu32u32_insert(&scope->variables, symbol.val, def.val);
        break;
    case SCOPE_KIND_TYPE:
        mapu32u32_insert(&scope->types, symbol.val, def.val);
        break;
    }
}

/// adds a used module to non-top-level anonymous scope
void hir_scope_anon_add_used_module(hir_scope_anon_t* scope, hir_def_id_t def_id) {
    assert(!scope->is_top_level); // handle this invariant
    if (!scope->has_used_defs) {
        const size_t hir_use_def_cap = 0x100;
        scope->used_hir_def_ids = vector_create_and_reserve(sizeof(hir_def_id_t), hir_use_def_cap);
        scope->has_used_defs = true;
    }
    vector_push_back(&scope->used_hir_def_ids, &def_id);
}

// ------------------------- named scope inserters -------------------------------
void hir_scope_insert_namespace(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_NAMESPACE);
}
void hir_scope_insert_function(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_FUNCTION);
}
void hir_scope_insert_variable(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_VARIABLE);
}
void hir_scope_insert_type(hir_scope_t* scope, hir_symbol_id_t symbol, hir_def_id_t def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_TYPE);
}

// -------------------------------------- anonymous scope inserters ----------------
void hir_scope_anon_insert_namespace(hir_scope_anon_t* scope, hir_symbol_id_t symbol,
                                     hir_def_id_t def) {
    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_NAMESPACE);
}
void hir_scope_anon_insert_function(hir_scope_anon_t* scope, hir_symbol_id_t symbol,
                                    hir_def_id_t def) {
    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_FUNCTION);
}
void hir_scope_anon_insert_variable(hir_scope_anon_t* scope, hir_symbol_id_t symbol,
                                    hir_def_id_t def) {
    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_VARIABLE);
}
void hir_scope_anon_insert_type(hir_scope_anon_t* scope, hir_symbol_id_t symbol, hir_def_id_t def) {

    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_TYPE);
}
