//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/scope.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/tables.hpp"
#include "utils/arena.h"
#include "utils/mapu32u32.h"
#include "utils/vector.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
// this may need to be tuned for a balance between cache locality and limited rehashing
#define HIR_SCOPE_MAP_DEFAULT_SIZE 0x800
#define HirScopeOP_LEVEL_SCALE_FACTOR 4
/// helper for looking into symbol -> def maps

namespace hir {

static inline DefId hir_symbol_to_def_map_look_up(hir_symbol_to_def_map_t* map,
                                                  SymbolId symbol_id) {
    const HirId* res = mapu32u32_cat(map, symbol_id.val());
    if (res == NULL) {
        return DefId{};
    }
    return DefId{*res};
}

void hir_scope_init_with_parent(Scope* scope, ScopeId parent, arena_t arena) {}

Scope::Scope(ScopeId parent, arena_t arena) {
    this->opt_parent = parent;
    this->arena = arena;
    this->functions = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    this->namespaces = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    this->variables = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    this->types = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
}
enum scope_kind_e : uint8_t {
    SCOPE_KIND_NAMESPACE,
    SCOPE_KIND_FUNCTION,
    SCOPE_KIND_VARIABLE,
    SCOPE_KIND_TYPE,
};

static inline ScopeLookUpResult hir_scope_look_up(HirTables* tables, ScopeId local_scope_id,
                                                  SymbolId symbol, scope_kind_e kind) {
    if (!local_scope_id.val()) {
        return ScopeLookUpResult{DefId{}, HIR_SCOPE_INVALID_SCOPE_SEARCHED};
    }
    // init curr scope local scope
    Scope* local_scope = static_cast<Scope*>(vector_at(&tables->scope_vec, local_scope_id.val()));
    Scope* curr_scope = local_scope;
    ScopeId curr_scope_id = local_scope_id;
    // begin search logic
    DefId def{};
    hir_scope_look_up_result_status status = HIR_SCOPE_LOOK_UP_OKAY;

    // start walking scopes from local thru parents
    while (!def.val()) {
        curr_scope
            = static_cast<decltype(curr_scope)>(vector_at(&tables->scope_vec, curr_scope_id.val()));
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
        if (def.val()) {
            break; // hit, stop now since we allow shadowing
        }
        const ScopeId parent_scope_id = curr_scope->opt_parent.as_id();
        assert((parent_scope_id.val() != curr_scope_id.val()) && "self-referential scope\n");
        curr_scope_id = parent_scope_id;
        if (!curr_scope_id.val()) {
            break; // no more parents, stop traversing
        }
    }
    if (!def.val()) {
        status = HIR_SCOPE_LOOK_UP_NOT_FOUND;
    }
    return ScopeLookUpResult{def, status};
}

ScopeLookUpResult hir_scope_look_up_namespace(HirTables* tables, ScopeId local_scope_id,
                                              SymbolId symbol) {
    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_NAMESPACE);
}
ScopeLookUpResult hir_scope_look_up_variable(HirTables* tables, ScopeId local_scope_id,
                                             SymbolId symbol) {
    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_VARIABLE);
}
ScopeLookUpResult hir_scope_look_up_function(HirTables* tables, ScopeId local_scope_id,
                                             SymbolId symbol) {

    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_FUNCTION);
}
ScopeLookUpResult hir_scope_look_up_type(HirTables* tables, ScopeId local_scope_id,
                                         SymbolId symbol) {

    return hir_scope_look_up(tables, local_scope_id, symbol, SCOPE_KIND_TYPE);
}

// ------------------ ANON SCOPE IMPL ----------------------

void hir_scope_anon_top_level_init(ScopeAnon* scope, arena_t arena) {
    scope->opt_named_parent = ScopeId{};
    scope->opt_anon_parent = ScopeAnonId{};
    scope->arena = arena;
    scope->variables = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HirScopeOP_LEVEL_SCALE_FACTOR, arena);
    scope->types = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HirScopeOP_LEVEL_SCALE_FACTOR, arena);
    // top-level WILL need this
    scope->functions = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HirScopeOP_LEVEL_SCALE_FACTOR, arena);

    scope->namespaces = mapu32u32_create_from_arena(
        (size_t)HIR_SCOPE_MAP_DEFAULT_SIZE * HirScopeOP_LEVEL_SCALE_FACTOR, arena);
    scope->is_top_level = true;
    scope->has_used_defs = false;
}

static inline void hir_scope_anon_init_impl(ScopeAnon* scope, ScopeId named_parent,
                                            ScopeAnonId anon_parent, arena_t arena) {
    scope->opt_named_parent = named_parent;
    scope->opt_anon_parent = anon_parent;
    scope->arena = arena;
    scope->variables = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    scope->types = mapu32u32_create_from_arena(HIR_SCOPE_MAP_DEFAULT_SIZE, arena);
    // ~~~~~~~ unused, so zero-init for more safety ~~~~~~~~~~~
    // since this scope is NOT the top level and is anonymous, it can NEVER contain function
    // (function scopes), struct/variant/union/module (namespace scopes).
    // We do however reuse the hir_scope_anon_t since the behavior is otherwise identical.
    scope->functions = (mapu32u32_t){.buckets = 0, .capacity = 0, .size = 0, .arena = {0}};
    scope->namespaces = scope->functions; // intentionally copy over the zero-init'd values
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    scope->is_top_level = false;
    scope->has_used_defs = false;
}
void hir_scope_anon_init_with_named_parent(ScopeAnon* scope, ScopeId named_parent, arena_t arena) {
    hir_scope_anon_init_impl(scope, named_parent, ScopeAnonId{}, arena);
}
void hir_scope_anon_init_with_anon_parent(ScopeAnon* scope, ScopeAnonId anon_parent,
                                          arena_t arena) {
    hir_scope_anon_init_impl(scope, ScopeId{}, anon_parent, arena);
}
void hir_scope_anon_destroy(ScopeAnon* scope) {
    if (scope->has_used_defs) {
        vector_destroy(&scope->used_hir_def_ids);
    }
}

static inline ScopeLookUpResult hir_scope_anon_look_up(HirTables* tables,
                                                       ScopeAnonId local_scope_id, SymbolId symbol,
                                                       scope_kind_e kind) {
    if (!local_scope_id.val()) {
        return ScopeLookUpResult{DefId{}, HIR_SCOPE_INVALID_SCOPE_SEARCHED};
    }
    // init curr scope local scope
    ScopeAnon* local_scope_anon = static_cast<decltype(local_scope_anon)>(
        vector_at(&tables->scope_anon_vec, local_scope_id.val()));
    ScopeAnon* curr_scope_anon = local_scope_anon;
    Scope* curr_scope_named = NULL;
    ScopeAnonId curr_scope_anon_id = local_scope_id;
    ScopeId curr_scope_named_id{};
    // begin search logic
    DefId result_def{};
    hir_scope_look_up_result_status status = HIR_SCOPE_LOOK_UP_OKAY;

    // search used modules first to allow local shadowing!
    DefId def_from_used_modules{};
    bool collision = false;
    if (local_scope_anon->has_used_defs) {

        assert(!local_scope_anon->is_top_level);

        vector_t* vec = &local_scope_anon->used_hir_def_ids;

        for (HirSize i = 0; i < vec->size; i++) {

            DefId def_id = *(DefId*)vector_at(vec, i);

            // TODO replace with encapsulated tables logic once the tables impl is written
            Def* used_def = (Def*)vector_at(&tables->def_vec, def_id.val());
            ScopeLookUpResult used_res = hir_scope_look_up(
                tables, std::get<DefModule>(used_def->value).scope, symbol, kind);

            if (used_res.status == HIR_SCOPE_LOOK_UP_OKAY) {
                if (def_from_used_modules.val()) {
                    collision = true;
                }
                def_from_used_modules = used_res.def_id;
            }
        }
    }

    // start walking scopes from local thru parents
    // exactly one of (curr_scope_anon_id.val, curr_scope_named_id.val) must be nonzero at every
    // step
    while (!result_def.val()) {
        assert(!(curr_scope_anon_id.val() && curr_scope_named_id.val()));

        if (curr_scope_anon_id.val()) {
            curr_scope_anon = static_cast<ScopeAnon*>(
                vector_at(&tables->scope_anon_vec, curr_scope_anon_id.val()));
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
            if (result_def.val()) {
                break; // hit, stop now since we allow shadowing
            }
            const ScopeAnonId parent_scope_anon_id = curr_scope_anon->opt_anon_parent.as_id();
            assert((parent_scope_anon_id.val() != curr_scope_anon_id.val())
                   && "self-referential scope\n");
            curr_scope_anon_id = parent_scope_anon_id;

            const ScopeId parent_scope_named_id = curr_scope_anon->opt_named_parent.as_id();
            assert((parent_scope_named_id.val() != curr_scope_named_id.val())
                   && "self-referential scope\n");
            curr_scope_named_id = parent_scope_named_id;
        } else {
            assert(curr_scope_named_id.val());
            curr_scope_named
                = static_cast<Scope*>(vector_at(&tables->scope_vec, curr_scope_named_id.val()));
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
            if (result_def.val()) {
                break; // hit, stop now since we allow shadowing
            }

            const ScopeId parent_scope_named_id = curr_scope_named->opt_parent.as_id();
            assert((parent_scope_named_id.val() != curr_scope_named_id.val())
                   && "self-referential scope\n");
            curr_scope_named_id = parent_scope_named_id;
            curr_scope_anon_id = ScopeAnonId{};
        }
        if (!curr_scope_anon_id.val() && !curr_scope_named_id.val()) {
            break; // no more parents, stop traversing
        }
    }
    if (!result_def.val()) {
        // didn't find a local symbol -> now check the imports
        if (collision) {
            status = HIR_SCOPE_LOOK_UP_COLLISION;
        } else if (def_from_used_modules.val() != HIR_ID_NONE) {
            result_def = def_from_used_modules;
            status = HIR_SCOPE_LOOK_UP_OKAY;
        } else {
            status = HIR_SCOPE_LOOK_UP_NOT_FOUND;
        }
    }
    return ScopeLookUpResult{result_def, status};
}

ScopeLookUpResult hir_scope_anon_look_up_namespace(HirTables* tables, ScopeAnonId local_scope,
                                                   SymbolId symbol) {
    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_NAMESPACE);
}
ScopeLookUpResult hir_scope_anon_look_up_variable(HirTables* tables, ScopeAnonId local_scope,
                                                  SymbolId symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_VARIABLE);
}
ScopeLookUpResult hir_scope_anon_look_up_function(HirTables* tables, ScopeAnonId local_scope,
                                                  SymbolId symbol) {
    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_FUNCTION);
}
ScopeLookUpResult hir_scope_anon_look_up_type(HirTables* tables, ScopeAnonId local_scope,
                                              SymbolId symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, SCOPE_KIND_TYPE);
}

// ------------------ insert helpers ----------------------

/// insert symbol -> def into a named hir_scope
static inline void hir_scope_insert(Scope* scope, SymbolId symbol, DefId def, scope_kind_e kind) {
    switch (kind) {
    case SCOPE_KIND_NAMESPACE:
        mapu32u32_insert(&scope->namespaces, symbol.val(), def.val());
        break;
    case SCOPE_KIND_FUNCTION:
        mapu32u32_insert(&scope->functions, symbol.val(), def.val());
        break;
    case SCOPE_KIND_VARIABLE:
        mapu32u32_insert(&scope->variables, symbol.val(), def.val());
        break;
    case SCOPE_KIND_TYPE:
        mapu32u32_insert(&scope->types, symbol.val(), def.val());
        break;
    }
}

/// insert symbol -> def into an anonymous hir_scope_anon
static inline void hir_scope_anon_insert(ScopeAnon* scope, SymbolId symbol, DefId def,
                                         scope_kind_e kind) {
    switch (kind) {
    case SCOPE_KIND_NAMESPACE:
        assert(scope->is_top_level); // only top-level anon scopes hold namespaces
        mapu32u32_insert(&scope->namespaces, symbol.val(), def.val());
        break;
    case SCOPE_KIND_FUNCTION:
        assert(scope->is_top_level); // only top-level anon scopes hold functions
        mapu32u32_insert(&scope->functions, symbol.val(), def.val());
        break;
    case SCOPE_KIND_VARIABLE:
        mapu32u32_insert(&scope->variables, symbol.val(), def.val());
        break;
    case SCOPE_KIND_TYPE:
        mapu32u32_insert(&scope->types, symbol.val(), def.val());
        break;
    }
}

/// adds a used module to non-top-level anonymous scope
void hir_scope_anon_add_used_module(ScopeAnon* scope, DefId def_id) {
    assert(!scope->is_top_level); // handle this invariant
    if (!scope->has_used_defs) {
        const size_t hir_use_def_cap = 0x100;
        scope->used_hir_def_ids = vector_create_and_reserve(sizeof(DefId), hir_use_def_cap);
        scope->has_used_defs = true;
    }
    vector_push_back(&scope->used_hir_def_ids, &def_id);
}

// ------------------------- named scope inserters -------------------------------
void hir_scope_insert_namespace(Scope* scope, SymbolId symbol, DefId def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_NAMESPACE);
}
void hir_scope_insert_function(Scope* scope, SymbolId symbol, DefId def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_FUNCTION);
}
void hir_scope_insert_variable(Scope* scope, SymbolId symbol, DefId def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_VARIABLE);
}
void hir_scope_insert_type(Scope* scope, SymbolId symbol, DefId def) {
    hir_scope_insert(scope, symbol, def, SCOPE_KIND_TYPE);
}

// -------------------------------------- anonymous scope inserters ----------------
void hir_scope_anon_insert_namespace(ScopeAnon* scope, SymbolId symbol, DefId def) {
    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_NAMESPACE);
}
void hir_scope_anon_insert_function(ScopeAnon* scope, SymbolId symbol, DefId def) {
    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_FUNCTION);
}
void hir_scope_anon_insert_variable(ScopeAnon* scope, SymbolId symbol, DefId def) {
    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_VARIABLE);
}
void hir_scope_anon_insert_type(ScopeAnon* scope, SymbolId symbol, DefId def) {

    hir_scope_anon_insert(scope, symbol, def, SCOPE_KIND_TYPE);
}

} // namespace hir
