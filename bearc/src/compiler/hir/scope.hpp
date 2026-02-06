//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_SCOPE_HPP
#define COMPILER_HIR_SCOPE_HPP

#include "compiler/hir/id_map.hpp"
#include "compiler/hir/indexing.hpp"
#include "utils/arena.h"
#include "utils/data_arena.hpp"
#include "utils/mapu32u32.h"
#include "utils/vector.h"
#include <cstdint>

namespace hir {

struct HirTables;

using ScopeIdMap = IdMap<SymbolId, DefId>;

typedef mapu32u32_t hir_symbol_to_def_map_t;

enum hir_scope_look_up_result_status : uint8_t {
    HIR_SCOPE_LOOK_UP_OKAY = 0,
    HIR_SCOPE_INVALID_SCOPE_SEARCHED,
    HIR_SCOPE_LOOK_UP_COLLISION,
    HIR_SCOPE_LOOK_UP_NOT_FOUND,
};

struct ScopeLookUpResult {
    DefId def_id;
    hir_scope_look_up_result_status status;
    constexpr ScopeLookUpResult(DefId def_id, hir_scope_look_up_result_status status)
        : def_id(def_id), status(status) {}
};
enum class scope_kind : uint8_t {
    NAMESPACE,
    FUNCTION,
    VARIABLE,
    TYPE,
};
/**
 * maps SymbolId -> hir_def_id
 * models named blocks/namespaces, such as function bodies or ctrl flow blocks
 */
struct Scope {
    OptId<ScopeId> named_parent;
    /// module, struct, and variant names
    ScopeIdMap namespaces;
    /// var foo;
    ScopeIdMap variables;
    /// top-level functions
    ScopeIdMap functions;
    /// structs, variants, unions, deftypes
    ScopeIdMap types;
    DataArena& arena;
    const bool top_level;
    void insert(SymbolId symbol, DefId def, scope_kind kind);

  public:
    bool is_top_level() const { return top_level; };
    Scope(ScopeId parent, DataArena& arena);
    Scope(ScopeId parent, DataArena& arena, bool is_top_level);
    static ScopeLookUpResult look_up_namespace(const HirTables& tables, ScopeId local_scope,
                                               SymbolId symbol);
    static ScopeLookUpResult look_up_variable(const HirTables& tables, ScopeId local_scope,
                                              SymbolId symbol);
    static ScopeLookUpResult look_up_function(const HirTables& tables, ScopeId local_scope,
                                              SymbolId symbol);
    static ScopeLookUpResult look_up_type(const HirTables& tables, ScopeId local_scope,
                                          SymbolId symbol);

    void insert_namespace(SymbolId symbol, DefId def);
    void insert_function(SymbolId symbol, DefId def);
    void insert_variable(SymbolId symbol, DefId def);
    void insert_type(SymbolId symbol, DefId def);

    friend class ScopeAnon;
};

/**
 * maps SymbolId -> hir_def_id
 * models anonymous blocks, such as function bodies or ctrl flow blocks
 */
struct ScopeAnon {
    OptId<ScopeId> opt_named_parent;
    OptId<ScopeAnonId> opt_anon_parent;
    /// structs, variants, unions, deftypes
    ScopeIdMap types;
    /// var foo;
    ScopeIdMap variables;
    /// modules, struct, and variant defs brought in
    vector_t used_hir_def_ids;
    /// for shared flat map storage
    DataArena& arena;
    // so that we can lazily init the used_defs_hir_def_id vector
    bool has_used_defs;

    void insert(SymbolId symbol, DefId def, scope_kind kind);

    friend class Scope;

  public:
    ScopeAnon(ScopeId named_parent, DataArena& arena);
    ScopeAnon(ScopeAnonId anon_parent, DataArena& arena);
    static ScopeLookUpResult look_up_variable(const HirTables& tables, ScopeAnonId local_scope,
                                              SymbolId symbol);
    static ScopeLookUpResult look_up_type(const HirTables& tables, ScopeAnonId local_scope,
                                          SymbolId symbol);

    /// adds a used module to non-top-level anonymous scope
    void add_used_module(DefId def_id);
    ~ScopeAnon();
    void insert_variable(SymbolId symbol, DefId def);
    void insert_type(SymbolId symbol, DefId def);
};

} // namespace hir

#endif
