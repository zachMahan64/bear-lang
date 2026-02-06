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
#include "utils/data_arena.hpp"
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
    /// const char* -> hir::SymbolId
    strimap_t str_to_symbol_id_map;

    std::vector<FileId> file_id_vec;
    std::vector<File> file_vec;
    IdMap<SymbolId, FileId> symbol_id_to_file_id_map;
    DataArena id_map_arena;

    std::vector<IdSlice<FileId>> importer_to_importees_vec;

    std::vector<IdSlice<FileId>> importee_to_importers_vec;

    std::vector<FileAst> ast_vec;

    std::vector<Scope> scopes;

    std::vector<ScopeAnon> scope_anons;

    std::vector<SymbolId> symbol_id_vec;
    std::vector<Symbol> symbol_vec;
    DataArena symbol_arena;

    std::vector<ExecId> exec_id_vec;
    std::vector<Exec> exec_vec;

    std::vector<DefId> def_id_vec;
    std::vector<Def> def_vec;

    /// indicated whether this node has been visited during top-level traversal/resolution; this
    /// flag helps prevent illegal circular dependencies
    std::vector<uint8_t> def_resolved;          // index with DefId
    std::vector<uint8_t> def_top_level_visited; // index with DefId

    std::vector<TypeId> type_id_vec;
    std::vector<Type> type_vec;

    std::vector<GenericParamId> generic_param_id_vec;
    std::vector<GenericParam> generic_param_vec;

    std::vector<GenericArgId> generic_arg_id_vec;
    std::vector<GenericArg> generic_arg_vec;

    HirTables();
};

} // namespace hir

#endif
