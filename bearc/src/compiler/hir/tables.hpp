//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TABLES_HPP
#define COMPILER_HIR_TABLES_HPP

#include "compiler/hir/def.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/hir/file.hpp"
#include "compiler/hir/generic.hpp"
#include "compiler/hir/id_map.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/type.hpp"
#include "utils/arena.h"
#include "utils/strimap.h"
#include <vector>

namespace hir {

// !! TODO, finish impl and improve impl (switch to entire STL vectors/other STL)

/**
 * primary data container for hir structures
 * - *_vec holds hir node data whereas *_id_vec holds simply ids pointing to hir nodes
 * - this model allows for slices with purely ids (no pointers)
 * - all vector tables MUST reserve id/idx 0 to store value 0!
 */
class HirTables {
  public:
    /// const char* -> hir_symbol_id_t
    strimap_t str_to_symbol_id_map;

    /// hir_file_id_idx_t -> hir_file_id_t
    std::vector<FileId> file_id_vec;
    /// hir_file_id_t -> hir_file_id_t
    std::vector<File> file_vec;
    /// hir_symbol_id_t -> hir_file_id_t
    IdMap<SymbolId, FileId> symbol_id_to_file_id_map;

    /// hir_file_id_t -> {hir_file_id_idx_t, hir_size_t}
    std::vector<IdSlice<FileId>> importer_to_importees_vec;

    /// hir_file_id_t -> {hir_file_id_idx_t, hir_size_t}
    std::vector<IdSlice<FileId>> importee_to_importers_vec;

    /// hir_ast_id -> ast_t
    std::vector<FileAst> ast_vec;

    /// hir_scope_id -> hir_scope_t
    std::vector<Scope> scopes;

    /// hir_scope_anon_id_t -> hir_scope_anon_t
    std::vector<ScopeAnon> scope_anons;

    /// hir_symbol_id_idx_t -> hir_symbol_id_t
    std::vector<SymbolId> symbol_id_vec;
    /// hir_symbol_id_t -> hir_symbol_t
    std::vector<Symbol> symbol_vec;
    /// stores interned strings pointed to by hir_symbol_id_t's
    arena_t symbol_arena;

    /// hir_exec_id_idx_t -> hir_exec_id_t
    std::vector<ExecId> exec_id_vec;
    /// hir_exec_id_t -> hir_exec_t
    std::vector<Exec> exec_vec;

    /// hir_def_id_idx_t -> hir_def_id_t
    std::vector<DefId> def_id_vec;
    /// hir_def_id_t -> hir_def_t
    std::vector<Def> def_vec;

    /// indicated whether this node has been visited during top-level traversal/resolution; this
    /// flag helps prevent illegal circular dependencies
    std::vector<uint8_t> def_resolved;          // index with DefId
    std::vector<uint8_t> def_top_level_visited; // index with DefId

    /// hir_type_id_idx_t -> hir_type_id_t
    std::vector<TypeId> type_id_vec;
    /// hir_type_id_t -> hir_type_t
    std::vector<Type> type_vec;

    /// hir_generic_param_id_idx_t -> hir_generic_param_id_t
    std::vector<GenericParamId> generic_param_id_vec;
    /// hir_generic_param_id_t -> hir_generic_param_t
    std::vector<GenericParam> generic_param_vec;

    /// hir_generic_arg_id_idx_t -> hir_generic_param_id_t
    std::vector<GenericArgId> generic_arg_id_vec;
    /// hir_generic_arg_id_t -> hir_generic_param_t
    std::vector<GenericArg> generic_arg_vec;

    HirTables();
    Scope scope_at();
};

} // namespace hir

#endif
