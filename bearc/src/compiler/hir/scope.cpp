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
#include "utils/data_arena.hpp"
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

static inline DefId hir_symbol_to_def_map_look_up(const hir_symbol_to_def_map_t* map,
                                                  SymbolId symbol_id) {
    const HirId* res = mapu32u32_cat(map, symbol_id.val());
    if (res == NULL) {
        return DefId{};
    }
    return DefId{*res};
}

Scope::Scope(ScopeId parent, DataArena& arena)
    : named_parent(parent), arena(arena), functions(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      namespaces(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), top_level(false) {}

Scope::Scope(ScopeId parent, DataArena& arena, bool is_top_level)
    : named_parent(parent), arena(arena), functions(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      namespaces(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), top_level(is_top_level) {}

static inline ScopeLookUpResult hir_scope_look_up(const HirTables& tables, ScopeId local_scope_id,
                                                  SymbolId symbol, scope_kind kind) {

    if (!local_scope_id.val()) {
        return ScopeLookUpResult{DefId{}, HIR_SCOPE_INVALID_SCOPE_SEARCHED};
    }
    // init curr scope local scope
    const Scope* local_scope = &tables.scopes.at(local_scope_id.val());
    const Scope* curr_scope = local_scope;
    ScopeId curr_scope_id = local_scope_id;
    // begin search logic
    DefId def{};
    hir_scope_look_up_result_status status = HIR_SCOPE_LOOK_UP_OKAY;

    // start walking scopes from local thru parents
    while (!def.val()) {
        curr_scope = &tables.scopes.at(curr_scope_id.val());
        switch (kind) {
        case scope_kind::NAMESPACE:
            def = curr_scope->namespaces.at(symbol).as_id();
            break;
        case scope_kind::FUNCTION:
            def = curr_scope->functions.at(symbol).as_id();
            break;
        case scope_kind::VARIABLE:
            def = curr_scope->variables.at(symbol).as_id();
            break;
        case scope_kind::TYPE:
            def = curr_scope->types.at(symbol).as_id();
            break;
        }
        if (def.val()) {
            break; // hit, stop now since we allow shadowing
        }
        const ScopeId parent_scope_id = curr_scope->named_parent.as_id();
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

ScopeLookUpResult Scope::look_up_namespace(const HirTables& tables, ScopeId local_scope,
                                           SymbolId symbol) {
    return hir_scope_look_up(tables, local_scope, symbol, scope_kind::NAMESPACE);
}
ScopeLookUpResult Scope::look_up_variable(const HirTables& tables, ScopeId local_scope,
                                          SymbolId symbol) {
    return hir_scope_look_up(tables, local_scope, symbol, scope_kind::VARIABLE);
}
ScopeLookUpResult Scope::look_up_function(const HirTables& tables, ScopeId local_scope,
                                          SymbolId symbol) {

    return hir_scope_look_up(tables, local_scope, symbol, scope_kind::FUNCTION);
}
ScopeLookUpResult Scope::look_up_type(const HirTables& tables, ScopeId local_scope,
                                      SymbolId symbol) {
    return hir_scope_look_up(tables, local_scope, symbol, scope_kind::TYPE);
}

// ------------------ ANON SCOPE IMPL ----------------------

static inline ScopeLookUpResult hir_scope_anon_look_up(const HirTables& tables,
                                                       ScopeAnonId local_scope_id, SymbolId symbol,
                                                       scope_kind kind) {
    if (!local_scope_id.val()) {
        return ScopeLookUpResult{DefId{}, HIR_SCOPE_INVALID_SCOPE_SEARCHED};
    }
    // init curr scope local scope
    const ScopeAnon* local_scope_anon = &tables.scope_anons.at(local_scope_id.val());
    const ScopeAnon* curr_scope_anon = local_scope_anon;
    const Scope* curr_scope_named = NULL;
    ScopeAnonId curr_scope_anon_id = local_scope_id;
    ScopeId curr_scope_named_id{};
    // begin search logic
    DefId result_def{};
    hir_scope_look_up_result_status status = HIR_SCOPE_LOOK_UP_OKAY;

    // search used modules first to allow local shadowing!
    DefId def_from_used_modules{};
    bool collision = false;
    if (local_scope_anon->has_used_defs) {

        const auto* vec = &local_scope_anon->used_hir_def_ids;

        for (HirSize i = 0; i < vec->size; i++) {

            const DefId def_id = *(DefId*)vector_at(vec, i);

            // TODO replace with encapsulated tables logic once the tables impl is written
            const Def& used_def = tables.def_vec.at(def_id.val());
            const ScopeLookUpResult used_res = hir_scope_look_up(
                tables, std::get<DefModule>(used_def.value).scope, symbol, kind);

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
            curr_scope_anon = &tables.scope_anons.at(curr_scope_anon_id.val());
            switch (kind) {
            case scope_kind::NAMESPACE:
            case scope_kind::FUNCTION:
                // functions decls only allowed at top-level/named
                assert(false && "namespace/function lookup in anonymous scope");
                break;
            case scope_kind::VARIABLE:
                result_def = curr_scope_anon->variables.at(symbol).as_id();
                break;
            case scope_kind::TYPE:
                result_def = curr_scope_anon->types.at(symbol).as_id();
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
            curr_scope_named = &tables.scopes.at(curr_scope_named_id.val());
            switch (kind) {
            case scope_kind::NAMESPACE:
                result_def = curr_scope_named->namespaces.at(symbol).as_id();
                break;
            case scope_kind::FUNCTION:
                result_def = curr_scope_named->functions.at(symbol).as_id();
                break;
            case scope_kind::VARIABLE:
                result_def = curr_scope_named->variables.at(symbol).as_id();
                break;
            case scope_kind::TYPE:
                result_def = curr_scope_named->types.at(symbol).as_id();
                break;
            }
            if (result_def.val()) {
                break; // hit, stop now since we allow shadowing
            }

            const ScopeId parent_scope_named_id = curr_scope_named->named_parent.as_id();
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

ScopeLookUpResult hir_scope_anon_look_up_variable(HirTables& tables, ScopeAnonId local_scope,
                                                  SymbolId symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, scope_kind::VARIABLE);
}

ScopeLookUpResult hir_scope_anon_look_up_type(HirTables& tables, ScopeAnonId local_scope,
                                              SymbolId symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, scope_kind::TYPE);
}

ScopeLookUpResult ScopeAnon::look_up_variable(const HirTables& tables, ScopeAnonId local_scope,
                                              SymbolId symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, scope_kind::VARIABLE);
}

ScopeLookUpResult ScopeAnon::look_up_type(const HirTables& tables, ScopeAnonId local_scope,
                                          SymbolId symbol) {

    return hir_scope_anon_look_up(tables, local_scope, symbol, scope_kind::TYPE);
}

// ------------------ insert helpers ----------------------

/// insert symbol -> def into a named hir_scope
void Scope::insert(SymbolId symbol, DefId def, scope_kind kind) {
    switch (kind) {
    case scope_kind::NAMESPACE:
        this->namespaces.insert(symbol, def);
        break;
    case scope_kind::FUNCTION:
        this->functions.insert(symbol, def);
        break;
    case scope_kind::VARIABLE:
        this->variables.insert(symbol, def);
        break;
    case scope_kind::TYPE:
        this->types.insert(symbol, def);
        break;
    }
}

/// insert symbol -> def into an anonymous hir_scope_anon
void ScopeAnon::insert(SymbolId symbol, DefId def, scope_kind kind) {
    switch (kind) {
    case scope_kind::NAMESPACE:
    case scope_kind::FUNCTION:
        assert(false && "namespace/function insertion in anonymous scope");
        break;
    case scope_kind::VARIABLE:
        this->variables.insert(symbol, def);
        break;
    case scope_kind::TYPE:
        this->types.insert(symbol, def);
        break;
    }
}

void ScopeAnon::add_used_module(DefId def_id) {
    // lazy init
    if (!this->has_used_defs) {
        const size_t hir_use_def_cap = 0x100;
        this->used_hir_def_ids = vector_create_and_reserve(sizeof(DefId), hir_use_def_cap);
        this->has_used_defs = true;
    }
    vector_push_back(&this->used_hir_def_ids, &def_id);
}
ScopeAnon::ScopeAnon(ScopeId named_parent, DataArena& arena)
    : opt_named_parent(named_parent), arena(arena), variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), has_used_defs(false) {}
ScopeAnon::ScopeAnon(ScopeAnonId anon_parent, DataArena& arena)
    : opt_anon_parent(anon_parent), arena(arena), variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), has_used_defs(false) {}

ScopeAnon::~ScopeAnon() {
    if (this->has_used_defs) {
        vector_destroy(&this->used_hir_def_ids);
    }
}

// ------------------------- named scope inserters -------------------------------
void Scope::insert_namespace(SymbolId symbol, DefId def) {
    insert(symbol, def, scope_kind::NAMESPACE);
}
void Scope::insert_function(SymbolId symbol, DefId def) {

    insert(symbol, def, scope_kind::FUNCTION);
}
void Scope::insert_variable(SymbolId symbol, DefId def) {
    insert(symbol, def, scope_kind::VARIABLE);
}
void Scope::insert_type(SymbolId symbol, DefId def) { insert(symbol, def, scope_kind::TYPE); }

// -------------------------------------- anonymous scope inserters ----------------
void ScopeAnon::insert_variable(SymbolId symbol, DefId def) {
    insert(symbol, def, scope_kind::VARIABLE);
}
void ScopeAnon::insert_type(SymbolId symbol, DefId def) { insert(symbol, def, scope_kind::TYPE); }

} // namespace hir
