//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
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

enum class hir_scope_look_up_result_status : uint8_t {
    OKAY = 0,
    SEARCHED,
    COLLISION,
    NOT_FOUND,
};

struct ScopeLookUpResult {
    DefId def_id;
    hir_scope_look_up_result_status status;
    constexpr ScopeLookUpResult(DefId def_id, hir_scope_look_up_result_status status)
        : def_id(def_id), status(status) {}
};
enum class scope_kind : uint8_t {
    NAMESPACE,
    VARIABLE,
    TYPE,
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
    static ScopeLookUpResult look_up_namespace(const Context& context, ScopeId local_scope,
                                               SymbolId symbol);
    static ScopeLookUpResult look_up_variable(const Context& context, ScopeId local_scope,
                                              SymbolId symbol);
    static ScopeLookUpResult look_up_type(const Context& context, ScopeId local_scope,
                                          SymbolId symbol);

    void insert_namespace(SymbolId symbol, DefId def);
    void insert_variable(SymbolId symbol, DefId def);
    void insert_type(SymbolId symbol, DefId def);

    friend class ScopeAnon;
};

/**
 * maps SymbolId -> hir_def_id
 * models anonymous blocks, such as function bodies or ctrl flow blocks
 */
struct ScopeAnon {
    using id_type = ScopeAnonId;
    OptId<ScopeId> opt_named_parent;
    OptId<ScopeAnonId> opt_anon_parent;
    /// structs, variants, unions, deftypes
    ScopeIdMap types;
    /// var foo;
    ScopeIdMap variables;
    static constexpr size_t NUM_HIGH_USED_DEFS = 16;
    /// modules, struct, and variant defs brought in
    llvm::SmallVector<DefId, NUM_HIGH_USED_DEFS> used_defs;
    /// for shared flat map storage
    DataArena& arena;
    // so that we can lazily init the used_defs_hir_def_id vector
    bool has_used_defs;

    void insert(SymbolId symbol, DefId def, scope_kind kind);
    static ScopeLookUpResult look_up_impl(const Context& context, ScopeAnonId local_scope_id,
                                          SymbolId symbol, scope_kind kind);
    friend class Scope;

  public:
    ScopeAnon(ScopeId named_parent, DataArena& arena);
    ScopeAnon(ScopeAnonId anon_parent, DataArena& arena);
    static ScopeLookUpResult look_up_variable(const Context& context, ScopeAnonId local_scope,
                                              SymbolId symbol);
    static ScopeLookUpResult look_up_type(const Context& context, ScopeAnonId local_scope,
                                          SymbolId symbol);

    /// adds a used module to non-top-level anonymous scope
    void add_used_module(DefId def_id);
    void insert_variable(SymbolId symbol, DefId def);
    void insert_type(SymbolId symbol, DefId def);
};

} // namespace hir

#endif
