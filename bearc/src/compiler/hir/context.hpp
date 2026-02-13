//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TABLES_HPP
#define COMPILER_HIR_TABLES_HPP

#include "cli/args.h"
#include "compiler/hir/arena_str_hash_map.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/hir/file.hpp"
#include "compiler/hir/generic.hpp"
#include "compiler/hir/id_hash_map.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/node_vector.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/token.h"
#include "utils/data_arena.hpp"
#include <atomic>

namespace hir {

/**
 * primary data container for hir structures
 * - this model allows for IDs and ID slices with no pointers
 */
class Context {
  public:
    // containers:
    // ~~~~~~~~~~~~~~~~~ file stuff ~~~~~~~~~~~~~~~~~~~
    IdVector<FileId> file_ids;
    NodeVector<File> files;
    DataArena id_map_arena;
    IdHashMap<SymbolId, FileId> symbol_id_to_file_id_map;
    NodeVector<FileAst> file_asts;

    IdVecMap<FileId, IdSlice<FileId>> importer_to_importees_vec;
    IdVecMap<FileId, IdSlice<FileId>> importee_to_importers_vec;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // ~~~~~~~~~~~~~~~~~~~~~ scopes ~~~~~~~~~~~~~~~~~~~~~~~
    NodeVector<Scope> scopes;
    NodeVector<ScopeAnon> scope_anons;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    /// const char* -> hir::SymbolId
    DataArena symbol_storage_arena;
    DataArena symbol_map_arena;
    StrIdHashMap<SymbolId> str_to_symbol_id_map;
    IdVector<SymbolId> symbol_ids;
    NodeVector<Symbol> symbols;

    IdVector<ExecId> exec_ids;
    NodeVector<Exec> execs;

    // ~~~~ defs ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    IdVector<DefId> def_ids;
    NodeVector<Def> defs;

    /// indicated whether this node has been visited during top-level traversal/resolution; this
    /// flag helps prevent illegal circular dependencies, TODO turn this into a single enum map
    IdVecMap<DefId, uint8_t> def_resolved;          // index with DefId
    IdVecMap<DefId, uint8_t> def_top_level_visited; // index with DefId
    /// tracks whether a defintion is used/unused (for tracking dead definitions)
    IdVecMap<DefId, uint8_t> def_used;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // types, generics ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    IdVector<TypeId> type_ids;
    NodeVector<Type> type_vec;

    IdVector<GenericParamId> generic_param_ids;
    NodeVector<GenericParam> generic_params;

    IdVector<GenericArgId> generic_arg_ids;
    NodeVector<GenericArg> generic_args;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // error tracking
    std::atomic<uint32_t> parse_error_count;
    std::atomic<uint32_t> semantic_error_count;
    // args
    const bearc_args_t* const args;
    Context(const bearc_args_t* args);
    void bump_parser_error_count(uint32_t cnt) noexcept;
    int error_count() const noexcept;

    // ----- accessors --------
    [[nodiscard]] SymbolId get_symbol_id(const char* start, size_t len);
    SymbolId get_symbol_id(std::string_view str);
    [[nodiscard]] FileId get_file(SymbolId path);

    // ----- info viewing ------
    const char* symbol_id_to_cstr(SymbolId id) const;

    // ------ transformers -----
    void explore_imports(FileId file_id);
    /// prints info based on cli-flags
    void try_print_info() const;

  private:
    FileId provide_root_file(const char* file_name);
    /// forceably emplaces ast, not checking if it has already been processed. This function is
    /// wrapped by file handling logic and should thus not be used directly anywhere else
    FileAstId emplace_ast(const char* file_name);
    [[nodiscard]] FileId get_file_from_path_tkn(token_t* tkn);
    [[nodiscard]] SymbolId get_symbol_id_for_tkn(token_t* tkn);
    /// get a symbol, trimming the "" quotes on the outside when interning
    [[nodiscard]] SymbolId get_symbol_id_for_str_lit_token(token_t* tkn);
};

} // namespace hir

#endif
