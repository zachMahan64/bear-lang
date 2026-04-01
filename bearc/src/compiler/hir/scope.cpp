//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/scope.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/indexing.hpp"
#include "utils/data_arena.hpp"
#include <assert.h>
#include <optional>
#include <stddef.h>
// this may need to be tuned for a balance between cache locality and limited rehashing
static constexpr size_t HIR_SCOPE_MAP_DEFAULT_SIZE = 0x100;
static constexpr size_t HirScopeOP_LEVEL_SCALE_FACTOR = 4;
/// helper for looking into symbol -> def maps

namespace hir {

Scope::Scope(ScopeId parent, DataArena& arena)
    : parent(parent), arena(arena), namespaces(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      top_level(false) {}

Scope::Scope(DataArena& arena)
    : arena(arena), namespaces(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      variables(arena, HIR_SCOPE_MAP_DEFAULT_SIZE), types(arena, HIR_SCOPE_MAP_DEFAULT_SIZE),
      top_level(true) {}

Scope::Scope(ScopeId parent, size_t capacity, DataArena& arena)
    : parent(parent), arena(arena), namespaces(arena, capacity), variables(arena, capacity),
      types(arena, capacity), top_level(false) {}

Scope::Scope(size_t capacity, DataArena& arena)
    : arena(arena), namespaces(arena, capacity), variables(arena, capacity), types(arena, capacity),
      top_level(true) {}

OptId<DefId> Scope::look_up_impl(const Context& context, ScopeId local_scope_id, SymbolId symbol,
                                 scope_kind kind) {

    if (!local_scope_id.val()) {
        return std::nullopt;
    }
    // init curr scope local scope
    const Scope* local_scope = &context.scope(local_scope_id);
    const Scope* curr_scope = local_scope;
    ScopeId curr_scope_id = local_scope_id;
    // begin search logic
    OptId<DefId> def{};

    // start walking scopes from local thru parents
    while (!def.val()) {
        curr_scope = &context.scope(curr_scope_id);
        switch (kind) {
        case scope_kind::namespacee:
            def = curr_scope->namespaces.at(symbol);
            break;
        case scope_kind::variable:
            def = curr_scope->variables.at(symbol);
            break;
        case scope_kind::type:
            def = curr_scope->types.at(symbol);
            break;
        }
        if (def.val()) {
            break; // hit, stop now since we allow shadowing
        }
        const OptId<ScopeId> parent_scope_id = curr_scope->parent;
        if (parent_scope_id.empty()) {
            break; // no more parents, stop traversing
        }
        curr_scope_id = parent_scope_id.as_id();
    }
    return def;
}

OptId<DefId> Scope::look_up_namespace(const Context& context, ScopeId local_scope,

                                      SymbolId symbol) {
    return look_up_impl(context, local_scope, symbol, scope_kind::namespacee);
}
OptId<DefId> Scope::look_up_variable(const Context& context, ScopeId local_scope, SymbolId symbol) {
    return look_up_impl(context, local_scope, symbol, scope_kind::variable);
}
OptId<DefId> Scope::look_up_type(const Context& context, ScopeId local_scope, SymbolId symbol) {
    return look_up_impl(context, local_scope, symbol, scope_kind::type);
}

// ------------------ insert helpers ----------------------

/// insert symbol -> def into a named hir_scope
void Scope::insert(SymbolId symbol, DefId def, scope_kind kind) {
    switch (kind) {
    case scope_kind::namespacee:
        this->namespaces.insert(symbol, def);
        break;
    case scope_kind::variable:
        this->variables.insert(symbol, def);
        break;
    case scope_kind::type:
        this->types.insert(symbol, def);
        break;
    }
}

void Scope::insert_namespace(SymbolId symbol, DefId def) {
    insert(symbol, def, scope_kind::namespacee);
}
void Scope::insert_variable(SymbolId symbol, DefId def) {
    insert(symbol, def, scope_kind::variable);
}
void Scope::insert_type(SymbolId symbol, DefId def) { insert(symbol, def, scope_kind::type); }

OptId<DefId> Scope::already_defines_variable(SymbolId symbol) const { return variables.at(symbol); }
OptId<DefId> Scope::already_defines_type(SymbolId symbol) const { return types.at(symbol); }
OptId<DefId> Scope::look_up_local_namespace(const Context& context, ScopeId local_scope,
                                            SymbolId symbol) {
    return context.scope(local_scope).namespaces.at(symbol);
}

static OptId<DefId> look_up_local_type(const Context& context, ScopeId local_scope,
                                       SymbolId symbol) {
    return context.scope(local_scope).types.at(symbol);
}

static OptId<DefId> look_up_local_variable(const Context& context, ScopeId local_scope,
                                           SymbolId symbol) {
    return context.scope(local_scope).variables.at(symbol);
}
} // namespace hir
