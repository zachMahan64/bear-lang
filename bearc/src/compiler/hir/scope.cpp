//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/scope.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/indexing.hpp"
#include "utils/data_arena.hpp"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
// this may need to be tuned for a balance between cache locality and limited rehashing
#define HIR_SCOPE_MAP_DEFAULT_SIZE 0x100
#define HirScopeOP_LEVEL_SCALE_FACTOR 4
/// helper for looking into symbol -> def maps

namespace hir {

Scope::Scope(ScopeId parent, DataArena& arena)
    : named_parent(parent), arena(arena), namespaces(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      top_level(false) {}

Scope::Scope(DataArena& arena)
    : arena(arena), namespaces(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      top_level(true) {}

ScopeLookUpResult Scope::look_up_impl(const Context& context, ScopeId local_scope_id,
                                      SymbolId symbol, scope_kind kind) {

    if (!local_scope_id.val()) {
        return ScopeLookUpResult{DefId{}, scope_look_up_status::searched};
    }
    // init curr scope local scope
    const Scope* local_scope = &context.scopes.cat(local_scope_id);
    const Scope* curr_scope = local_scope;
    ScopeId curr_scope_id = local_scope_id;
    // begin search logic
    DefId def{};
    scope_look_up_status status = scope_look_up_status::okay;

    // start walking scopes from local thru parents
    while (!def.val()) {
        curr_scope = &context.scopes.cat(curr_scope_id);
        switch (kind) {
        case scope_kind::NAMESPACE:
            def = curr_scope->namespaces.at(symbol).as_id();
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
        status = scope_look_up_status::not_found;
    }
    return ScopeLookUpResult{def, status};
}

ScopeLookUpResult Scope::look_up_namespace(const Context& context, ScopeId local_scope,
                                           SymbolId symbol) {
    return look_up_impl(context, local_scope, symbol, scope_kind::NAMESPACE);
}
ScopeLookUpResult Scope::look_up_variable(const Context& context, ScopeId local_scope,
                                          SymbolId symbol) {
    return look_up_impl(context, local_scope, symbol, scope_kind::VARIABLE);
}
ScopeLookUpResult Scope::look_up_type(const Context& context, ScopeId local_scope,
                                      SymbolId symbol) {
    return look_up_impl(context, local_scope, symbol, scope_kind::TYPE);
}

// ------------------ ANON SCOPE IMPL ----------------------

ScopeLookUpResult ScopeAnon::look_up_impl(const Context& context, ScopeAnonId local_scope_id,
                                          SymbolId symbol, scope_kind kind) {
    if (!local_scope_id.val()) {
        return ScopeLookUpResult{DefId{}, scope_look_up_status::searched};
    }
    // init curr scope local scope
    const ScopeAnon* local_scope_anon = &context.scope_anons.cat(local_scope_id);
    const ScopeAnon* curr_scope_anon = local_scope_anon;
    const Scope* curr_scope_named = NULL;
    ScopeAnonId curr_scope_anon_id = local_scope_id;
    ScopeId curr_scope_named_id{};
    // begin search logic
    DefId result_def{};
    scope_look_up_status status = scope_look_up_status::okay;

    // search used modules first to allow local shadowing!
    DefId def_from_used_modules{};
    bool collision = false;
    if (local_scope_anon->has_used_defs) {

        const auto& vec = local_scope_anon->used_defs;

        for (HirSize i = 0; i < vec.size(); i++) {

            const DefId def_id = vec[i];

            // TODO replace with encapsulated context logic once the tables impl is written
            const Def& used_def = context.defs.cat(def_id);
            const ScopeLookUpResult used_res = Scope::look_up_impl(
                context, std::get<DefModule>(used_def.value).scope, symbol, kind);

            if (used_res.status == scope_look_up_status::okay) {
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
            curr_scope_anon = &context.scope_anons.cat(curr_scope_anon_id);
            switch (kind) {
            case scope_kind::NAMESPACE:
                assert(false && "namespace lookup in anonymous scope");
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
            curr_scope_named = &context.scopes.cat(curr_scope_named_id);
            switch (kind) {
            case scope_kind::NAMESPACE:
                result_def = curr_scope_named->namespaces.at(symbol).as_id();
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
            status = scope_look_up_status::collision;
        } else if (def_from_used_modules.val() != HIR_ID_NONE) {
            result_def = def_from_used_modules;
            status = scope_look_up_status::okay;
        } else {
            status = scope_look_up_status::not_found;
        }
    }
    return ScopeLookUpResult{result_def, status};
}

ScopeLookUpResult hir_scope_anon_look_up_variable(Context& context, ScopeAnonId local_scope,
                                                  SymbolId symbol) {

    return ScopeAnon::look_up_impl(context, local_scope, symbol, scope_kind::VARIABLE);
}

ScopeLookUpResult hir_scope_anon_look_up_type(Context& context, ScopeAnonId local_scope,
                                              SymbolId symbol) {

    return ScopeAnon::look_up_impl(context, local_scope, symbol, scope_kind::TYPE);
}

ScopeLookUpResult ScopeAnon::look_up_variable(const Context& context, ScopeAnonId local_scope,
                                              SymbolId symbol) {

    return ScopeAnon::look_up_impl(context, local_scope, symbol, scope_kind::VARIABLE);
}

ScopeLookUpResult ScopeAnon::look_up_type(const Context& context, ScopeAnonId local_scope,
                                          SymbolId symbol) {

    return ScopeAnon::look_up_impl(context, local_scope, symbol, scope_kind::TYPE);
}

// ------------------ insert helpers ----------------------

/// insert symbol -> def into a named hir_scope
void Scope::insert(SymbolId symbol, DefId def, scope_kind kind) {
    switch (kind) {
    case scope_kind::NAMESPACE:
        this->namespaces.insert(symbol, def);
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
        assert(false && "namespace insertion in anonymous scope");
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
        const size_t hir_use_def_cap = 0x10;
        this->used_defs.reserve(hir_use_def_cap);
        this->has_used_defs = true;
    }
    this->used_defs.push_back(def_id);
}
ScopeAnon::ScopeAnon(ScopeId named_parent, DataArena& arena)
    : opt_named_parent(named_parent), arena(arena), variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), has_used_defs(false) {}
ScopeAnon::ScopeAnon(ScopeAnonId anon_parent, DataArena& arena)
    : opt_anon_parent(anon_parent), arena(arena), variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), has_used_defs(false) {}

// ------------------------- named scope inserters -------------------------------
void Scope::insert_namespace(SymbolId symbol, DefId def) {
    insert(symbol, def, scope_kind::NAMESPACE);
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

OptId<DefId> Scope::already_defines_variable(SymbolId symbol) const { return variables.at(symbol); }
OptId<DefId> Scope::already_defines_type(SymbolId symbol) const { return types.at(symbol); }

OptId<DefId> ScopeAnon::already_defines_variable(SymbolId symbol) const {
    return variables.at(symbol);
}
OptId<DefId> ScopeAnon::already_defines_type(SymbolId symbol) const { return types.at(symbol); }

} // namespace hir
