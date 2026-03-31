//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_SCOPE_HPP
#define COMPILER_HIR_SCOPE_HPP

#include "compiler/hir/id_hash_map.hpp"
#include "compiler/hir/indexing.hpp"
#include "utils/data_arena.hpp"
#include "llvm/ADT/SmallVector.h"
#include <cstdint>

namespace hir {

struct Context;

using ScopeIdMap = IdHashMap<SymbolId, DefId>;

enum class scope_look_up_status : uint8_t {
    okay = 0,
    searched,
    collision,
    not_found,
};

struct ScopeLookUpResult {
    DefId def_id;
    scope_look_up_status status;
    constexpr ScopeLookUpResult(DefId def_id, scope_look_up_status status) noexcept
        : def_id(def_id), status(status) {}
};
enum class scope_kind : uint8_t {
    namespacee,
    variable,
    type,
};
/**
 * maps SymbolId -> hir_def_id
 * models named blocks/namespaces, such as function bodies or ctrl flow blocks
 */
struct Scope {
    using id_type = ScopeId;
    OptId<ScopeId> named_parent;
    /// module, struct, and variant names
    ScopeIdMap namespaces;
    /// var foo;
    ScopeIdMap variables;
    /// structs, variants, unions, deftypes
    ScopeIdMap types;
    DataArena& arena;
    const bool top_level;
    void insert(SymbolId symbol, DefId def, scope_kind kind);
    static ScopeLookUpResult look_up_impl(const Context& context, ScopeId local_scope_id,
                                          SymbolId symbol, scope_kind kind);

  public:
    bool is_top_level() const { return top_level; };
    // constructs a non-top-level scope with a parent
    Scope(ScopeId parent, DataArena& arena);
    // constructs a top level scope
    Scope(DataArena& arena);
    // constructs a non-top-level scope with a given capacity parent
    Scope(ScopeId parent, size_t capacity, DataArena& arena);
    // constructs a top level scope with a given capacity
    Scope(size_t capacity, DataArena& arena);
    static ScopeLookUpResult look_up_namespace(const Context& context, ScopeId local_scope,
                                               SymbolId symbol);
    static ScopeLookUpResult look_up_variable(const Context& context, ScopeId local_scope,
                                              SymbolId symbol);
    static ScopeLookUpResult look_up_type(const Context& context, ScopeId local_scope,
                                          SymbolId symbol);

    void insert_namespace(SymbolId symbol, DefId def);
    void insert_variable(SymbolId symbol, DefId def);
    void insert_type(SymbolId symbol, DefId def);

    OptId<DefId> already_defines_variable(SymbolId symbol) const;
    OptId<DefId> already_defines_type(SymbolId symbol) const;
    // looks up a namespace without checking parent scopes for any definitons
    static OptId<DefId> look_up_local_namespace(const Context& context, ScopeId local_scope,
                                                SymbolId symbol);
    static OptId<DefId> look_up_local_type(const Context& context, ScopeId local_scope,
                                           SymbolId symbol);
    static OptId<DefId> look_up_local_variable(const Context& context, ScopeId local_scope,
                                               SymbolId symbol);

    friend class ScopeAnon;
};

} // namespace hir

#endif
