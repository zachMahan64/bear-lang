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
#include "compiler/ast/stmt.h"
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
#include "llvm/ADT/SmallVector.h"
#include <atomic>
#include <cstdint>
#include <filesystem>

namespace hir {

/**
 * primary data container for hir structures
 * - this model allows for IDs and ID slices with no pointers
 */
class Context {
  public:
    Context(const bearc_args_t* args);
    int error_count() const noexcept;
    // ----- accessors --------
    [[nodiscard]] SymbolId get_symbol_id(const char* start, size_t len);
    [[nodiscard]] SymbolId get_symbol_id(std::string_view str);
    [[nodiscard]] FileId get_file(SymbolId path);
    [[nodiscard]] FileId get_file(std::filesystem::path& path);
    [[nodiscard]] const char* file_name(FileId id) const;
    [[nodiscard]] const FileAst& ast(FileId file_id) const;
    [[nodiscard]] ScopeId get_top_level_scope();
    [[nodiscard]] ScopeId make_named_scope();

    /// for registering definitions at the top level before resolution
    DefId register_top_level_def(SymbolId name, bool pub, Span span, ast_stmt_t* stmt);

    // ----- info viewing ------
    const char* symbol_id_to_cstr(SymbolId id) const;

    // ------ transformers -----
    void explore_imports(FileId root_id);
    void explore_imports(FileId importer_file_id, llvm::SmallVectorImpl<FileId>& import_stack);
    /// prints info based on cli-flags
    void try_print_info() const;

    friend class Scope;
    friend class ScopeAnon;
    friend class AstVisitor;

  private:
    // containers:
    // ~~~~~~~~~~~~~~~~~ file stuff ~~~~~~~~~~~~~~~~~~~
    IdVector<FileId> file_ids;
    NodeVector<File> files;
    DataArena id_map_arena;
    IdHashMap<SymbolId, FileId> symbol_id_to_file_id_map;
    NodeVector<FileAst> file_asts;

    /// FileId -> IdSlice<FileId> since all importees are always known when lowering a given file
    IdVecMap<FileId, IdSlice<FileId>> importer_to_importees;
    /// FileId -> llvm::SmallVector<FileId> since this will be updated less predictably
    IdVecMap<FileId, llvm::SmallVector<FileId>> importee_to_importers;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // ~~~~~~~~~~~~~~~~~~~~~ scopes ~~~~~~~~~~~~~~~~~~~~~~~
    DataArena scope_arena;
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

    /// indicated whether this node is unvisited, visited during top-level resolution, or resolved
    IdVecMap<DefId, Def::resol_state> def_resol_states; // index with DefId
    /// cached dense mapping of DefIds to AST nodes for fast resolution, this mapping should never
    /// be serialized
    IdVecMap<DefId, ast_stmt_t*> def_ast_nodes;
    /// tracks whether a defintion is used/unused/modified (for tracking dead definitions)
    IdVecMap<DefId, Def::mention_state> def_mention_states;
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
    std::atomic<uint32_t> hard_error_count;
    std::atomic<uint32_t> semantic_error_count;
    std::atomic<uint32_t> fatal_error_count;

    // args
    const bearc_args_t* const args;
    void bump_parser_error_count(uint32_t cnt) noexcept;

    FileId provide_root_file(const char* file_name);
    /// forceably emplaces ast, not checking if it has already been processed. This function is
    /// wrapped by file handling logic and should thus not be used directly anywhere else
    FileAstId emplace_ast(const char* file_name);
    [[nodiscard]] SymbolId get_symbol_id_for_tkn(token_t* tkn);
    /// get a symbol, trimming the "" quotes on the outside when interning
    [[nodiscard]] SymbolId get_symbol_id_for_str_lit_token(token_t* tkn);
    void register_importer(FileId importee, FileId importer);
    void report_cycle(FileId cyclical_file_id, llvm::SmallVectorImpl<FileId>& import_stack) const;
    void register_tokenwise_error(FileId file_id, token_t* tkn, error_code_e error_code);
    [[nodiscard]] OptId<FileId> try_file_from_import_statement(FileId importer_id,
                                                               const ast_stmt_t* import_statement);
};

} // namespace hir

#endif
