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

// !! TODO: impl accessor logic

/**
 * primary data container for hir structures
 * - this model allows for IDs and ID slices with no pointers
 */
class HirTables {
  public:
    // containers:
    // ~~~~~~~~~~~~~~~~~ file stuff ~~~~~~~~~~~~~~~~~~~
    IdVector<FileId> file_ids;
    NodeVector<File> files;
    IdHashMap<SymbolId, FileId> symbol_id_to_file_id_map;
    DataArena id_map_arena;
    NodeVector<FileAst> file_asts;

    IdVecMap<FileId, IdSlice<FileId>> importer_to_importees_vec;
    IdVecMap<FileId, IdSlice<FileId>> importee_to_importers_vec;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // ~~~~~~~~~~~~~~~~~~~~~ scopes ~~~~~~~~~~~~~~~~~~~~~~~
    NodeVector<Scope> scopes;
    NodeVector<ScopeAnon> scope_anons;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    /// const char* -> hir::SymbolId
    strimap_t str_to_symbol_id_map; // todo write a c++-style strimap (arena-backed string hash map)
    IdVector<SymbolId> symbol_ids;
    NodeVector<Symbol> symbols;
    DataArena symbol_arena;

    IdVector<ExecId> exec_ids;
    NodeVector<Exec> execs;

    // ~~~~ defs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    IdVector<DefId> def_ids;
    NodeVector<Def> defs;

    /// indicated whether this node has been visited during top-level traversal/resolution; this
    /// flag helps prevent illegal circular dependencies
    IdVecMap<DefId, uint8_t> def_resolved;          // index with DefId
    IdVecMap<DefId, uint8_t> def_top_level_visited; // index with DefId
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // types, generics ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    IdVector<TypeId> type_ids;
    NodeVector<Type> type_vec;

    IdVector<GenericParamId> generic_param_ids;
    NodeVector<GenericParam> generic_params;

    IdVector<GenericArgId> generic_arg_ids;
    NodeVector<GenericArg> generic_args;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    HirTables();
};

} // namespace hir

#endif
