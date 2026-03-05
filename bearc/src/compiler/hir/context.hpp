//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_TABLES_HPP
#define COMPILER_HIR_TABLES_HPP

#include "cli/args.h"
#include "compiler/ast/stmt.h"
#include "compiler/hir/arena_str_hash_map.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/exec.hpp"
#include "compiler/hir/file.hpp"
#include "compiler/hir/generic.hpp"
#include "compiler/hir/id_hash_map.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/node_vector.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/top_level_def_visitor.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/token.h"
#include "utils/data_arena.hpp"
#include "llvm/ADT/SmallVector.h"
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
    int diagnostic_count() const noexcept;
    int error_count() const noexcept;
    int warning_count() const noexcept;
    int note_count() const noexcept;
    // ----- accessors / emplacers --------
    [[nodiscard]] SymbolId symbol_id(const token_t* tkn);
    [[nodiscard]] SymbolId symbol_id(const char* start, size_t len);
    [[nodiscard]] SymbolId symbol_id(std::string_view str);
    [[nodiscard]] FileId file(SymbolId path);
    [[nodiscard]] FileId file(std::filesystem::path& path);
    [[nodiscard]] const char* file_name(FileId id) const;
    [[nodiscard]] FileAst& ast(FileId file_id);
    [[nodiscard]] const FileAst& ast(FileId file_id) const;

    // ------ scoping -----------
    [[nodiscard]] ScopeId root_scope();
    [[nodiscard]] ScopeId make_named_scope(OptId<ScopeId> parent_scope = OptId<ScopeId>{});
    // makes a named scope with a small capacity
    [[nodiscard]] ScopeId make_small_named_scope(OptId<ScopeId> parent_scope);
    [[nodiscard]] Scope& scope(ScopeId scope);
    [[nodiscard]] OptId<DefId> look_up_variable(NamedOrAnonScopeId scope, SymbolId sid) const;
    [[nodiscard]] OptId<DefId> look_up_type(NamedOrAnonScopeId scope, SymbolId sid) const;
    [[nodiscard]] OptId<DefId> look_up_namespace(NamedOrAnonScopeId scope, SymbolId sid) const;

    /// record ordered definitions to be frozen as an IdSlice<DefId> corresponding to a DefId
    /// (particularly requiring ordered members, like a struct)
    void register_ordered_defs(DefId def, llvm::SmallVectorImpl<DefId>& vec);
    /// indicates resolution state of a definition
    Def::resol_state resol_state_of(DefId def) const;
    IdSlice<SymbolId> symbol_slice(token_ptr_slice_t token_slice);
    /// finds a variable and attempts to resolve definitions on the way to it
    OptId<DefId> find_variable_from_scoped_id(NamedOrAnonScopeId scope, IdSlice<SymbolId> id_slice);

    // diagnostics
    void handle_bump_diag_counts(diag_code code, diag_type type);
    DiagnosticId emplace_diagnostic(Span span, diag_code code, diag_type type,
                                    OptId<DiagnosticId> next = OptId<DiagnosticId>{});
    DiagnosticId emplace_diagnostic(Span span, diag_code code, diag_type type,
                                    DiagnosticValue value,
                                    OptId<DiagnosticId> next = OptId<DiagnosticId>{});
    void set_next_diagnostic(DiagnosticId diag, DiagnosticId next);
    void print_diagnostic(DiagnosticId diag);
    // type emplacer
    /// emplaces and gets the id from a new CanonicalTypeId corresponding to a TypeId which points
    /// to the first structural representation of the canonical type
    [[nodiscard]] CanonicalTypeId
    emplace_and_get_canonical_type_id(TypeId first_structural_type_id);

    /// accessfor for a def thru a DefId
    Def& def(DefId def_id);
    FileId def_to_file_id(DefId def) const;
    Span make_def_name_span(DefId def, const ast_stmt_t* stmt) const;
    Span make_top_level_def_name_span(DefId def) const;

    bool is_top_level_def_with_associated_scope(DefId def_id) const;
    /// gets the named scope for a def
    ScopeId scope_for_top_level_def(DefId def);

    /// for registering definitions at the top level before resolution
    DefId register_top_level_def(SymbolId name, bool pub, bool compt, bool statik, bool generic,
                                 Span span, ast_stmt_t* stmt, OptId<DefId> parent = OptId<DefId>{});

    // ----- info viewing ------
    const char* symbol_id_to_cstr(SymbolId id) const;

    // ------ transformers -----
    void explore_imports(FileId root_id);
    void explore_imports(FileId importer_file_id, llvm::SmallVectorImpl<FileId>& import_stack,
                         const token_t* import_path_tkn);
    /// prints info based on cli-flags
    void try_print_info();

    // converters
    [[nodiscard]] FileId file_id(IdIdx<FileId> ididx) const;
    [[nodiscard]] const Type& type(IdIdx<TypeId> ididx) const;
    [[nodiscard]] Type& type(IdIdx<TypeId> ididx);
    [[nodiscard]] const Type& type(TypeId id) const;
    [[nodiscard]] Type& type(TypeId id);
    [[nodiscard]] TypeId type_id(IdIdx<TypeId> tid) const;
    [[nodiscard]] const Def& def(DefId id) const;
    [[nodiscard]] const Def& def(IdIdx<DefId> id) const;
    [[nodiscard]] const Exec& exec(ExecId id) const;
    [[nodiscard]] const Exec& exec(IdIdx<ExecId> id) const;

    friend class Scope;
    friend class ScopeAnon;
    friend class FileAstVisitor;
    friend class TopLevelDefVisitor;
    friend class TopLevelConstantExprSolver;

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
    // maps a FileId to a list of diagnostics
    IdVecMap<FileId, llvm::SmallVector<DiagnosticId>> file_to_diagnostics;
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

    /// for tracking DefId -> ScopeId for structs during the top level resolution
    IdHashMap<DefId, ScopeId> def_to_scope_for_types;

    // for storing ordered defs (namely ordered member definitions)
    IdVecMap<OrderedDefSliceId, IdSlice<DefId>> ordered_def_slices;
    // for accessing ordered def slices corresponding to a given DefId
    IdHashMap<DefId, OrderedDefSliceId> def_to_ordered_def_slice;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    // types, generics ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    IdVector<TypeId> type_ids;
    NodeVector<Type> types;

    // maps a canonical type back to its first TypeId mention so the type's structure can be rebuilt
    // even if only its canonical value is known
    IdVecMap<CanonicalTypeId, TypeId> canonical_to_type_id;
    DataArena canonical_type_table_arena;
    CanonicalTypeTable canonical_type_table;

    IdVector<GenericParamId> generic_param_ids;
    NodeVector<GenericParam> generic_params;

    IdVector<GenericArgId> generic_arg_ids;
    NodeVector<GenericArg> generic_args;
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // error tracking

    NodeVector<Diagnostic> diagnostics;
    IdVecMap<DiagnosticId, uint8_t> diagnostics_used;
    HirSize warning_cnt{};
    HirSize note_cnt{};
    HirSize normal_error_cnt{};
    HirSize fatal_error_cnt{};

    // args
    const bearc_args_t* const args;

    FileId provide_root_file(const char* file_name);
    /// forceably emplaces ast, not checking if it has already been processed. This function is
    /// wrapped by file handling logic and should thus not be used directly anywhere else
    FileAstId emplace_ast(const char* file_name);
    [[nodiscard]] SymbolId get_symbol_id_for_tkn(const token_t* tkn);
    /// get a symbol, trimming the "" quotes on the outside when interning
    [[nodiscard]] SymbolId get_symbol_id_for_str_lit_token(token_t* tkn);
    void register_importer(FileId importee, FileId importer);
    void report_cycle(FileId cyclical_file_id, llvm::SmallVectorImpl<FileId>& import_stack,
                      const token_t* import_path_tkn);
    [[nodiscard]] OptId<FileId> try_file_from_import_statement(FileId importer_id,
                                                               const ast_stmt_t* import_statement);
};

} // namespace hir

#endif
