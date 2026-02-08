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
#include "compiler/hir/id_hash_map.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/node_vector.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/type.hpp"
#include "utils/data_arena.hpp"
#include "utils/strimap.h"

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
    /// const char* -> hir::SymbolId
    strimap_t str_to_symbol_id_map;

    IdVector<FileId> file_ids;
    NodeVector<File> files;
    IdHashMap<SymbolId, FileId> symbol_id_to_file_id_map;
    DataArena id_map_arena;

    IdVecMap<FileId, IdSlice<FileId>> importer_to_importees_vec;

    IdVecMap<FileId, IdSlice<FileId>> importee_to_importers_vec;

    NodeVector<FileAst> file_asts;

    NodeVector<Scope> scopes;

    NodeVector<ScopeAnon> scope_anons;

    IdVector<SymbolId> symbol_id_vec;
    NodeVector<Symbol> symbols;
    DataArena symbol_arena;

    IdVector<ExecId> exec_id_vec;
    NodeVector<Exec> execs;

    IdVector<DefId> def_id_vec;
    NodeVector<Def> defs;

    /// indicated whether this node has been visited during top-level traversal/resolution; this
    /// flag helps prevent illegal circular dependencies
    IdVecMap<DefId, uint8_t> def_resolved;          // index with DefId
    IdVecMap<DefId, uint8_t> def_top_level_visited; // index with DefId

    IdVector<TypeId> type_id_vec;
    NodeVector<Type> type_vec;

    IdVector<GenericParamId> generic_param_id_vec;
    NodeVector<GenericParam> generic_param_vec;

    IdVector<GenericArgId> generic_arg_id_vec;
    NodeVector<GenericArg> generic_arg_vec;

    HirTables();
};

} // namespace hir

#endif
