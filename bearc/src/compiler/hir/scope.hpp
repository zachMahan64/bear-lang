//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_SCOPE_HPP
#define COMPILER_HIR_SCOPE_HPP

#include "compiler/hir/indexing.hpp"
#include "utils/arena.h"
#include "utils/mapu32u32.h"
#include "utils/vector.h"
#include <cstdint>

namespace hir {

struct HirTables;

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

/**
 * maps SymbolId -> hir_def_id
 * models named blocks/namespaces, such as function bodies or ctrl flow blocks
 */
struct Scope {
    ScopeId opt_parent;
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
    Scope(ScopeId parent, arena_t arena);
};

ScopeLookUpResult hir_scope_look_up_namespace(HirTables* tables, ScopeId local_scope,
                                              SymbolId symbol);
ScopeLookUpResult hir_scope_look_up_variable(HirTables* tables, ScopeId local_scope,
                                             SymbolId symbol);
ScopeLookUpResult hir_scope_look_up_function(HirTables* tables, ScopeId local_scope,
                                             SymbolId symbol);
ScopeLookUpResult hir_scope_look_up_type(HirTables* tables, ScopeId local_scope, SymbolId symbol);

void hir_scope_insert_namespace(Scope* scope, SymbolId symbol, DefId def);
void hir_scope_insert_function(Scope* scope, SymbolId symbol, DefId def);
void hir_scope_insert_variable(Scope* scope, SymbolId symbol, DefId def);
void hir_scope_insert_type(Scope* scope, SymbolId symbol, DefId def);

/**
 * maps SymbolId -> hir_def_id
 * models anonymous blocks, such as function bodies or ctrl flow blocks
 */
typedef struct hir_scope_anon {
    OptId<ScopeId> opt_named_parent;
    OptId<ScopeAnonId> opt_anon_parent;
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
} ScopeAnon;

void hir_scope_anon_top_level_init(ScopeAnon* scope, arena_t arena);
void hir_scope_anon_init_with_named_parent(ScopeAnon* scope, ScopeId named_parent, arena_t arena);
void hir_scope_anon_init_with_anon_parent(ScopeAnon* scope, ScopeAnonId anon_parent, arena_t arena);
void hir_scope_anon_destroy(ScopeAnon* scope);

ScopeLookUpResult hir_scope_anon_look_up_namespace(HirTables* tables, ScopeAnonId local_scope,
                                                   SymbolId symbol);
ScopeLookUpResult hir_scope_anon_look_up_variable(HirTables* tables, ScopeAnonId local_scope,
                                                  SymbolId symbol);
ScopeLookUpResult hir_scope_anon_look_up_function(HirTables* tables, ScopeAnonId local_scope,
                                                  SymbolId symbol);
ScopeLookUpResult hir_scope_anon_look_up_type(HirTables* tables, ScopeAnonId local_scope,
                                              SymbolId symbol);
void hir_scope_anon_insert_namespace(ScopeAnon* scope, SymbolId symbol, DefId def);
void hir_scope_anon_insert_function(ScopeAnon* scope, SymbolId symbol, DefId def);
void hir_scope_anon_insert_variable(ScopeAnon* scope, SymbolId symbol, DefId def);
void hir_scope_anon_insert_type(ScopeAnon* scope, SymbolId symbol, DefId def);
/// adds a used module to non-top-level anonymous scope
void hir_scope_anon_add_used_module(ScopeAnon* scope, DefId def_id);

} // namespace hir

#endif
