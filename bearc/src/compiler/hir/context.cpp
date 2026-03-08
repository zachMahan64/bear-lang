//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/context.hpp"
#include "cli/args.h"
#include "cli/import_path.h"
#include "compiler/ast/printer.h"
#include "compiler/ast/stmt.h"
#include "compiler/hir/ast_visitor.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/file.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/span.hpp"
#include "compiler/hir/top_level_def_visitor.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/log.hpp"
#include "llvm/ADT/SmallVector.h"
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <iso646.h>
#include <stddef.h>
#include <string_view>
#include <variant>
namespace hir {

static constexpr size_t DEFAULT_SYMBOL_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_SCOPE_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_CANONICAL_TYPE_ARENA_CAP = 0x1000;
static constexpr size_t DEFAULT_ID_MAP_ARENA_CAP
    = 0x8000; // increase if any other top level maps need to be made
static constexpr size_t DEFAULT_SYM_TO_FILE_ID_MAP_CAP = 0x80;
static constexpr size_t DEFAULT_SCOPE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_AST_VEC_CAP = DEFAULT_FILE_VEC_CAP;
static constexpr size_t DEFAULT_FILE_ID_VEC_CAP = 0x200;
static constexpr size_t DEFAULT_SCOPE_ANON_VEC_CAP = 0x100;
static constexpr size_t DEFAULT_SYMBOL_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_EXEC_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_DEF_CAP = 0x800;
static constexpr size_t DEFAULT_TYPE_VEC_CAP = 0x400;
static constexpr size_t DEFAULT_CANONICAL_TYPE_VEC_CAP = 0x100;
static constexpr size_t DEFAULT_GENERIC_PARAM_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_GENERIC_ARG_VEC_CAP = 0x400;
static constexpr HirSize EXPECTED_HIGH_NUM_IMPORTS = 128;
static constexpr size_t DEFAULT_DIAG_NUM = 0x100;
static constexpr size_t DEFAULT_DEF_SLICE_COUNT = 0x100;
static constexpr size_t DEFAULT_CANONICAL_TT_CAP = 0x400;

Context::Context(const bearc_args_t* args)
    : symbol_storage_arena{DEFAULT_SYMBOL_ARENA_CAP}, id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
      symbol_id_to_file_id_map{id_map_arena, DEFAULT_SYM_TO_FILE_ID_MAP_CAP},
      scopes{DEFAULT_SCOPE_VEC_CAP}, files{DEFAULT_FILE_VEC_CAP},
      file_asts{DEFAULT_FILE_AST_VEC_CAP}, scope_anons{DEFAULT_SCOPE_ANON_VEC_CAP},
      symbols{DEFAULT_SYMBOL_VEC_CAP}, execs{DEFAULT_EXEC_VEC_CAP}, defs{DEFAULT_DEF_CAP},
      file_ids{DEFAULT_FILE_ID_VEC_CAP}, importer_to_importees{DEFAULT_FILE_VEC_CAP},
      importee_to_importers{DEFAULT_FILE_VEC_CAP}, symbol_ids{DEFAULT_SYMBOL_VEC_CAP},
      exec_ids{DEFAULT_EXEC_VEC_CAP}, def_ids{DEFAULT_DEF_CAP}, def_resol_states{DEFAULT_DEF_CAP},
      def_mention_states{DEFAULT_DEF_CAP}, types{DEFAULT_TYPE_VEC_CAP}, type_ids{DEFAULT_DEF_CAP},
      generic_param_ids{DEFAULT_GENERIC_PARAM_VEC_CAP},
      generic_params{DEFAULT_GENERIC_PARAM_VEC_CAP}, generic_arg_ids{DEFAULT_GENERIC_ARG_VEC_CAP},
      generic_args{DEFAULT_GENERIC_ARG_VEC_CAP}, symbol_map_arena{DEFAULT_SYMBOL_ARENA_CAP},
      str_to_symbol_id_map{symbol_map_arena}, args{args}, scope_arena{DEFAULT_SCOPE_ARENA_CAP},
      def_ast_nodes(DEFAULT_DEF_CAP), diagnostics{DEFAULT_DIAG_NUM},
      diagnostics_used{DEFAULT_DIAG_NUM}, file_to_diagnostics{EXPECTED_HIGH_NUM_IMPORTS},
      def_to_scope_for_types{id_map_arena, DEFAULT_DEF_CAP},
      def_to_ordered_def_slice_id{id_map_arena, DEFAULT_DEF_SLICE_COUNT},
      ordered_def_slices{DEFAULT_DEF_CAP}, canonical_to_type_id(DEFAULT_CANONICAL_TYPE_VEC_CAP),
      canonical_type_table_arena{DEFAULT_CANONICAL_TYPE_ARENA_CAP},
      canonical_type_table(*this, canonical_type_table_arena, DEFAULT_CANONICAL_TT_CAP) {

    // this may only fail in horribly malfored arguments in test cases
    assert(args->input_file_name);

    // get try to get root file, and allow checking cwd for it

    std::optional<std::filesystem::path> maybe_root_file
        = resolve_on_import_path(args->input_file_name, ".", args);

    if (!maybe_root_file) {
        return;
    }

    const auto& root_file = maybe_root_file.value();

    FileId root_id = provide_root_file(root_file.c_str());

    // search imports to build all asts
    this->explore_imports(root_id);
    // tally parser errors
    for (FileId id = files.rbegin_id(); id != files.rend_id(); --id) {
        File& f = files.at(id);
        const FileAst& ast = file_asts.cat(f.ast_id);
        if (!args->flags[CLI_FLAG_PARSE_ONLY]) {
            FileAstVisitor visitor{*this, id};
            visitor.register_top_level_declarations();
        }
        this->note_cnt += ast.diagnostic_count() - ast.error_count();
        this->fatal_error_cnt += ast.error_count();
    }
    if (args->flags[CLI_FLAG_PARSE_ONLY]) {
        return;
    }
    TopLevelDefVisitor{*this}.resolve_top_level_definitions();
}

int Context::diagnostic_count() const noexcept {
    return static_cast<int>(fatal_error_cnt + normal_error_cnt + note_cnt + warning_cnt);
}

int Context::error_count() const noexcept {
    return static_cast<int>(this->fatal_error_cnt + this->normal_error_cnt);
}

int Context::warning_count() const noexcept { return static_cast<int>(warning_cnt); }
int Context::note_count() const noexcept { return static_cast<int>(note_cnt); }

SymbolId Context::symbol_id(std::string_view str) { return symbol_id(str.data(), str.length()); }
SymbolId Context::symbol_id(const token_t* tkn) { return symbol_id(tkn->start, tkn->len); }
SymbolId Context::symbol_id(const char* start, size_t len) {
    OptId<SymbolId> maybe_symbol = str_to_symbol_id_map.atn(start, len);
    if (maybe_symbol.has_value()) {
        return maybe_symbol.as_id();
    }
    char* sym_data = this->symbol_storage_arena.alloc_as<char*>(len + 1); // +1 for null-term
    memcpy(sym_data, start, len);
    sym_data[len] = '\0'; // null-term

    // make sym
    SymbolId sym_id = this->symbols.emplace_and_get_id(std::string_view{sym_data, len});
    // register sym into map for future fast and cosistent access
    str_to_symbol_id_map.emplace(sym_data, sym_id);
    // give sym_id now that it's fully interned
    return sym_id;
}
SymbolId Context::symbol_id_for_identifier_tkn(const token_t* tkn) {
    assert(tkn->type == TOK_IDENTIFIER || tkn->type == TOK_SELF_ID);
    return symbol_id(tkn->start, tkn->len);
}
SymbolId Context::symbol_id_for_str_lit_tkn(const token_t* tkn) {
    assert(tkn->type == TOK_STR_LIT);
    return symbol_id(tkn->start + 1, tkn->len - 2); // trims outer quotes
}

FileId Context::provide_root_file(const char* file_name) {
    SymbolId path_symbol = symbol_id(file_name, strlen(file_name));
    FileId file_id = file(path_symbol);
    return file_id;
}

FileId Context::file(SymbolId path_symbol) {
    // check if file has already been requested
    OptId<FileId> maybe_file_id = symbol_id_to_file_id_map.at(path_symbol);
    if (maybe_file_id.has_value()) {
        return maybe_file_id.as_id();
    }
    FileAstId ast_id = this->file_asts.emplace_and_get_id(symbol_id_to_cstr(path_symbol));
    FileId file_id = this->files.emplace_and_get_id(path_symbol, ast_id);
    /// store this mapping for future detection
    this->symbol_id_to_file_id_map.insert(path_symbol, file_id);
    /// bump necessary things that are track id-wise for files
    importee_to_importers.bump();
    importer_to_importees.bump();
    file_to_diagnostics.bump();
    return file_id;
}

FileId Context::file(std::filesystem::path& path) {
    SymbolId symbol = symbol_id(path.c_str());
    return file(symbol);
}
FileAstId Context::emplace_ast(const char* file_name) {
    return this->file_asts.emplace_and_get_id(file_name);
}

const char* Context::symbol_id_to_cstr(SymbolId id) const {
    return this->symbols.cat(id).sv().data();
}
/**
 * updates the slice storing the importers for an importee
 */
void Context::register_importer(FileId importee, FileId importer) {
    importee_to_importers.at(importee).emplace_back(importer);
}

void Context::report_cycle(FileId cyclical_file_id, llvm::SmallVectorImpl<FileId>& import_stack,
                           const token_t* import_path_tkn) {
    // gonna be imported in the previous thing, so top of import stack
    FileId imported_in = import_stack[import_stack.size() - 1];
    emplace_diagnostic(Span{imported_in, ast(imported_in).buffer(), import_path_tkn},
                       diag_code::cyclical_import, diag_type::error,
                       DiagnosticImportStack{freeze_id_vec(import_stack)});
}
void Context::explore_imports(FileId root_id) {
    llvm::SmallVector<FileId> import_stack{};
    import_stack.push_back(root_id);
    explore_imports(root_id, import_stack, nullptr);
}

void Context::explore_imports(FileId importer_file_id, llvm::SmallVectorImpl<FileId>& import_stack,
                              const token_t* import_path_tkn) {
    auto& file = files.at(importer_file_id);

    // angry base case, guard circularity
    if (file.load_state == file_import_state::in_progress) {
        // safe to take id because a circularity is only possible if a prev id exists
        report_cycle(importer_file_id, import_stack, import_path_tkn);
        return;
    }
    // happy base case
    if (file.load_state == file_import_state::done) {
        return;
    }

    const FileAst& root_ast = this->file_asts.at(this->files.at(importer_file_id).ast_id);
    const ast_stmt* root = root_ast.root();
    if (!root) {
        ERR("ast.root() == nullptr");
        return;
    }

    // register this file as in progress to track potential circularity
    file.load_state = file_import_state::in_progress;

    // for tracking importees
    llvm::SmallVector<FileId, EXPECTED_HIGH_NUM_IMPORTS> importees;

    for (size_t i = 0; i < root->stmt.file.stmts.len; i++) {
        ast_stmt* curr = root->stmt.file.stmts.start[i];
        if (curr->type == AST_STMT_IMPORT) {
            OptId<FileId> maybe_importee_file_id
                = this->try_file_from_import_statement(importer_file_id, curr);
            if (!maybe_importee_file_id.has_value()) {
                continue;
            }
            FileId importee_file_id = maybe_importee_file_id.as_id();
            // add importee to vec
            importees.emplace_back(importee_file_id);

            // add to reverse dependency list
            this->register_importer(importee_file_id, importer_file_id);

            // recursively traverse
            import_stack.push_back(importer_file_id);
            const token_t* tkn = curr->stmt.import.file_path;
            this->explore_imports(importee_file_id, import_stack, tkn);
            import_stack.pop_back();
        }
    }

    // set forward dependency list
    auto importer_to_importees_slice = freeze_id_vec(importees);

    importer_to_importees.at(importer_file_id) = importer_to_importees_slice;

    file.load_state = file_import_state::done;
}

void Context::try_print_info() {

    // 1. try print out ast-wise information (token tables, pretty-printing)
    for (auto fid = files.begin_id(); fid != files.end_id(); fid++) {
        ast(fid).try_print_info(args);
    }
    // 2. print more info:
    if (args->flags[CLI_FLAG_LIST_FILES]) {
        std::cout << ansi_bold_white() << "all files" << '(' << files.size() << ')' << ":"
                  << ansi_reset() << '\n';
        for (FileId curr = files.begin_id(); curr != files.end_id(); ++curr) {

            const FileAst& ast = file_asts.cat(files.cat(curr).ast_id);

            std::cout << ansi_bold_white() << '[' << curr.val() << "] " << ast.file_name();
            const auto list = importer_to_importees.cat(
                symbol_id_to_file_id_map.at(files.cat(curr).path).as_id());

            if (list.len() != 0) {
                std::cout << ": ";
            }
            for (auto imp = list.first(); imp != list.end(); ++imp) {
                if (imp.val() == 0) {
                    continue;
                }
                FileId importee = file_ids.cat(imp); // file_ids.cat(imp);
                std::cout << '[' << importee.val() << "] "
                          << symbol_id_to_cstr(files.cat(importee).path);
                // do this check to avoid trailing comma
                if (imp.val() != list.end().val() - 1) {
                    std::cout << ", ";
                }
            }
            std::cout << ansi_reset() << "\n";
        }
    }
    // 3. print diagnostics last (so always seen first in terminal)
    if (!args->flags[CLI_FLAG_SILENT]) {
        // go thru each file ast to print info
        for (auto fid = files.begin_id(); fid != files.end_id(); fid++) {
            const FileAst& aast = ast(fid);
            // 1. print parse-time errors (ast-wise errors)
            aast.print_all_errors();
            // 2. print diagnostics (semantic/non-grammatical errors)
            for (const auto d : file_to_diagnostics.cat(fid)) {
                print_diagnostic(d);
            }
        }
    }
    if (!args->flags[CLI_FLAG_SILENT]) {
        auto errors = error_count();
        if (errors == 1) {
            puts("1 error generated.");
        } else if (errors != 0) {
            printf("%d errors generated.\n", errors);
        }
        auto warnings = warning_count();
        if (warnings == 1) {
            puts("1 warning generated.");
        } else if (warnings != 0) {
            printf("%d warnings generated.\n", warnings);
        }
        auto notes = note_count();
        if (notes == 1) {
            puts("1 note generated.");
        } else if (notes != 0) {
            printf("%d notes generated.\n", notes);
        }
    }
    // std::cout << tables.files.size() << '\n';
    if (this->diagnostic_count() != 0) {
        if (!args->flags[CLI_FLAG_SILENT]) {
            printf("compilation terminated: %s'%s'\n%s", ansi_bold_white(),
                   symbol_id_to_cstr(files.cat(FileId{1}).path), ansi_reset());
        }
    }
    // release printer's internal state
    pretty_printer_reset();
}

const char* Context::file_name(FileId id) const { return symbol_id_to_cstr(files.cat(id).path); }

OptId<FileId> Context::try_file_from_import_statement(FileId importer_id,
                                                      const ast_stmt_t* import_statement) {
    assert(import_statement->type == AST_STMT_IMPORT);
    token_t* path_tkn = import_statement->stmt.import.file_path;
    SymbolId path_symbol_id = symbol_id_for_str_lit_tkn(path_tkn);
    const char* path = symbol_id_to_cstr(path_symbol_id);

    const std::filesystem::path parent = std::filesystem::path(path).parent_path();

    // DNE guard
    auto maybe_path = resolve_on_import_path(path, parent, this->args);
    if (!maybe_path.has_value()) {
        // register_tokenwise_error(importer_id, path_tkn, ERR_IMPORTED_FILE_DOES_NOT_EXIST);
        emplace_diagnostic(Span(importer_id, ast(importer_id).buffer(), path_tkn),
                           diag_code::imported_file_dne, diag_type::error);
        return OptId<FileId>{};
    }
    return file(maybe_path.value());
}

DefId Context::register_top_level_def(SymbolId name, bool pub, bool compt, bool statik,
                                      bool generic, Span span, ast_stmt_t* stmt,
                                      OptId<DefId> parent) {
    DefId def = defs.emplace_and_get_id(DefUnevaluated{}, name, pub, compt, statik, generic, span,
                                        parent);
    def_resol_states.bump(Def::resol_state::top_level_visited);
    def_ast_nodes.bump(stmt);
    def_mention_states.bump(Def::mention_state::unmentioned);
    return def;
}

FileAst& Context::ast(FileId file_id) { return file_asts.at(files.at(file_id).ast_id); }

const FileAst& Context::ast(FileId file_id) const {
    return file_asts.cat(files.cat(file_id).ast_id);
}

ScopeId Context::get_or_make_root_scope() {
    if (scopes.size() == 0) {
        return scopes.emplace_and_get_id(scope_arena);
    }
    // the top-level scope will have to be the first scope!
    return ScopeId{1};
}

ScopeId Context::root_scope() const {
    // the top-level scope will have to be the first scope!
    assert(scopes.size() != 0 && "tried to get the root scope before its creation");
    return ScopeId{1};
}

ScopeId Context::make_named_scope(OptId<ScopeId> parent_scope) {
    return (parent_scope.has_value()) ? scopes.emplace_and_get_id(parent_scope.as_id(), scope_arena)
                                      : scopes.emplace_and_get_id(scope_arena);
}

ScopeId Context::make_small_named_scope(OptId<ScopeId> parent_scope) {
    static constexpr size_t CAP = 0x8;
    return (parent_scope.has_value())
               ? scopes.emplace_and_get_id(parent_scope.as_id(), CAP, scope_arena)
               : scopes.emplace_and_get_id(CAP, scope_arena);
}

Scope& Context::scope(ScopeId scope) { return scopes.at(scope); }

void Context::handle_bump_diag_counts(diag_code code, diag_type type) {
    if (type == diag_type::error) {
        switch (code) {
        case diag_code::imported_file_dne:
            fatal_error_cnt++;
            break;
        default:
            normal_error_cnt++;
            break;
        }
    } else if (type == diag_type::note) {
        note_cnt++;
    } else if (type == diag_type::warning) {
        warning_cnt++;
    }
}

DiagnosticId Context::emplace_diagnostic(Span span, diag_code code, diag_type type,
                                         OptId<DiagnosticId> next) {
    DiagnosticId id = diagnostics.emplace_and_get_id(span, code, type, next);
    diagnostics_used.bump();
    file_to_diagnostics.at(span.file_id).emplace_back(id);
    handle_bump_diag_counts(code, type);
    return id;
}

DiagnosticId Context::emplace_diagnostic(Span span, diag_code code, diag_type type,
                                         DiagnosticValue value, OptId<DiagnosticId> next) {
    DiagnosticId id = diagnostics.emplace_and_get_id(span, code, type, value, next);
    diagnostics_used.bump();
    file_to_diagnostics.at(span.file_id).emplace_back(id);
    handle_bump_diag_counts(code, type);
    return id;
}

void Context::print_diagnostic(DiagnosticId diag_id) {
    // as to not re-report
    if (diagnostics_used.cat(diag_id)) {
        return;
    }
    const Diagnostic& diag = diagnostics.cat(diag_id);
    diag.print(*this);
    // mark used
    diagnostics_used.at(diag_id) = true;
    // try print next
    if (diag.next.has_value() && !diagnostics_used.cat(diag.next.as_id())) {
        print_diagnostic(diag.next.as_id());
    }
}

void Context::set_next_diagnostic(DiagnosticId diag, DiagnosticId next) {
    diagnostics.at(diag).set_next(next);
}

Def& Context::def(DefId def_id) { return defs.at(def_id); }

FileId Context::file_id(IdIdx<FileId> ididx) const { return file_ids.cat(ididx); }

void Context::register_ordered_defs(DefId def, llvm::SmallVectorImpl<DefId>& vec) {
    IdSlice<DefId> def_slice = freeze_id_vec(vec);
    OrderedDefSliceId ord_def_slice_id = ordered_def_slices.emplace_and_get_id(def_slice);
    def_to_ordered_def_slice_id.insert(def, ord_def_slice_id);
}

IdSlice<DefId> Context::ordered_defs_for(DefId def) {
    auto maybe_odef_slice_id = def_to_ordered_def_slice_id.at(def);
    if (!maybe_odef_slice_id.has_value()) {
        // empty slice
        return IdSlice<DefId>{};
    }
    return ordered_def_slices.cat(maybe_odef_slice_id.as_id());
}

Def::resol_state Context::resol_state_of(DefId def) const { return def_resol_states.cat(def); }

void Context::set_resol_state_of(DefId def, Def::resol_state resol_state) {
    def_resol_states.at(def) = resol_state;
}

ScopeId Context::scope_for_top_level_def(DefId def_id) const {
    auto hopefully_scope = try_scope_for_top_level_def(def_id);
    if (hopefully_scope.has_value()) {
        return hopefully_scope.as_id();
    }
    ERR(def(def_id).span.as_sv(*this));
    assert(false && "tried to get a scope for a top level def that was incompatible");
    return root_scope();
}

OptId<ScopeId> Context::try_scope_for_top_level_def(DefId def_id) const {
    const auto& def_node = defs.cat(def_id);
    // no parent means parent scope is root scope
    if (!def_node.parent.has_value()) {
        return root_scope();
    }
    if (def_node.holds<DefModule>()) {
        return def_node.as<DefModule>().scope;
    }
    if (is_top_level_def_with_associated_scope(def_id)) {
        OptId<Id<Scope>> hopefully_scope = def_to_scope_for_types.at(def_id);
        if (hopefully_scope.has_value()) {
            return hopefully_scope.as_id();
        }
    }
    return OptId<ScopeId>{};
}

bool Context::is_top_level_def_with_associated_scope(DefId def_id) const {
    ast_stmt_type_e decl_type = def_ast_nodes.cat(def_id)->type;
    return decl_type == AST_STMT_VARIANT_DEF || decl_type == AST_STMT_CONTRACT_DEF
           || decl_type == AST_STMT_STRUCT_DEF || decl_type == AST_STMT_UNION_DEF;
}

FileId Context::def_to_file_id(DefId def) const { return defs.cat(def).span.file_id; }

Span Context::make_def_name_span(DefId def, const ast_stmt_t* stmt) const {
    auto fid = def_to_file_id(def);
    auto maybe_name = FileAstVisitor::name_of_ast_decl(stmt);
    if (!maybe_name.has_value()) {
        assert(false && "failed to get name for an AST declaration");
    }
    return Span(fid, ast(fid).buffer(), maybe_name.value());
}

Span Context::make_top_level_def_name_span(DefId def) const {
    return make_def_name_span(def, def_ast_nodes.cat(def));
}

const Type& Context::type(IdIdx<TypeId> ididx) const { return types.cat(type_ids.cat(ididx)); }
Type& Context::type(IdIdx<TypeId> ididx) { return types.at(type_ids.at(ididx)); }
const Type& Context::type(TypeId id) const { return types.cat(id); }
Type& Context::type(TypeId id) { return types.at(id); }
TypeId Context::type_id(IdIdx<TypeId> tid) const { return type_ids.cat(tid); }
CanonicalTypeId Context::emplace_and_get_canonical_type_id(TypeId first_structural_type_id) {
    return canonical_to_type_id.emplace_and_get_id(first_structural_type_id);
}

TypeId Context::emplace_type(const TypeValue& value, Span span, bool mut) {
    TypeId tid = types.emplace_and_get_id(value, span, mut);
    // set canonical
    types.at(tid).canonical = canonical_type_table.canonical(tid);
    return tid;
}
[[nodiscard]] const Exec& Context::exec(ExecId id) const { return execs.cat(id); }

[[nodiscard]] const Def& Context::def(DefId id) const { return defs.cat(id); }

[[nodiscard]] const Exec& Context::exec(IdIdx<ExecId> id) const {
    return execs.cat(exec_ids.cat(id));
}
[[nodiscard]] const Def& Context::def(IdIdx<DefId> id) const { return defs.cat(def_ids.cat(id)); }

[[nodiscard]] DefId Context::def_id(IdIdx<DefId> id) const { return def_ids.cat(id); }

OptId<DefId> Context::look_up_variable(NamedOrAnonScopeId scope, SymbolId sid) const {
    if (std::holds_alternative<ScopeAnonId>(scope)) {
        auto res = hir::ScopeAnon::look_up_variable(*this, std::get<ScopeAnonId>(scope), sid);
        return (res.status == scope_look_up_status::okay) ? res.def_id : OptId<DefId>{};
    }
    auto res = hir::Scope::look_up_variable(*this, std::get<ScopeId>(scope), sid);
    return (res.status == scope_look_up_status::okay) ? res.def_id : OptId<DefId>{};
}
OptId<DefId> Context::look_up_type(NamedOrAnonScopeId scope, SymbolId sid) const {
    if (std::holds_alternative<ScopeAnonId>(scope)) {
        auto res = hir::ScopeAnon::look_up_type(*this, std::get<ScopeAnonId>(scope), sid);
        return (res.status == scope_look_up_status::okay) ? res.def_id : OptId<DefId>{};
    }
    auto res = hir::Scope::look_up_type(*this, std::get<ScopeId>(scope), sid);
    return (res.status == scope_look_up_status::okay) ? res.def_id : OptId<DefId>{};
}
OptId<DefId> Context::look_up_namespace(NamedOrAnonScopeId scope, SymbolId sid) const {
    if (std::holds_alternative<ScopeAnonId>(scope)) {
        auto res = hir::ScopeAnon::look_up_namespace(*this, std::get<ScopeAnonId>(scope), sid);
        return (res.status == scope_look_up_status::okay) ? res.def_id : OptId<DefId>{};
    }
    auto res = hir::Scope::look_up_variable(*this, std::get<ScopeId>(scope), sid);
    return (res.status == scope_look_up_status::okay) ? res.def_id : OptId<DefId>{};
}

OptId<DefId> Context::look_up_scoped_variable(NamedOrAnonScopeId scope,
                                              IdSlice<SymbolId> id_slice) {
    NamedOrAnonScopeId curr_scope = scope;
    for (IdIdx<SymbolId> sidx = id_slice.begin(); sidx != id_slice.end(); sidx++) {
        SymbolId sid = symbol_ids.cat(sidx);
        // base case, last elem should be the variable
        if (sidx == id_slice.last_elem()) {
            return look_up_variable(curr_scope, sid);
        }
        if (auto maybe_mod = look_up_namespace(curr_scope, sid); maybe_mod.has_value()) {
            curr_scope = def(maybe_mod.as_id()).as<DefModule>().scope;
        } else if (auto maybe_type = look_up_type(curr_scope, sid); maybe_type.has_value()) {
            curr_scope = scope_for_top_level_def(maybe_type.as_id());
        }
    }
    // never entered the loop, so not found
    return OptId<DefId>{};
}

OptId<DefId> Context::look_up_scoped_type(NamedOrAnonScopeId scope, IdSlice<SymbolId> id_slice) {
    NamedOrAnonScopeId curr_scope = scope;
    for (IdIdx<SymbolId> sidx = id_slice.begin(); sidx != id_slice.end(); sidx++) {
        SymbolId sid = symbol_ids.cat(sidx);
        // base case, last elem should be the variable
        if (sidx == id_slice.last_elem()) {
            return look_up_type(curr_scope, sid);
        }
        if (auto maybe_mod = look_up_namespace(curr_scope, sid); maybe_mod.has_value()) {
            curr_scope = def(maybe_mod.as_id()).as<DefModule>().scope;
        }
        // yes, since nested structs are allowed
        else if (auto maybe_type = look_up_type(curr_scope, sid); maybe_type.has_value()) {
            curr_scope = scope_for_top_level_def(maybe_type.as_id());
        }
    }
    // never entered the loop, so not found
    return OptId<DefId>{};
}

IdSlice<SymbolId> Context::symbol_slice(token_ptr_slice_t token_slice) {
    llvm::SmallVector<SymbolId> vec{};
    for (size_t i = 0; i < token_slice.len; i++) {
        const token_t* tkn = token_slice.start[i];
        vec.push_back(symbol_id(tkn));
    }
    return freeze_id_vec(vec);
}

NamedOrAnonScopeId Context::containing_scope(DefId did) const {
    auto maybe_parent = defs.cat(did).parent;
    // ----- base cases (parent w/ scope)---------
    if (!maybe_parent.has_value()) {
        return root_scope();
    }
    const Def& defi = def(maybe_parent.as_id());
    if (defi.holds<DefModule>()) {
        return defi.as<DefModule>().scope;
    }
    if (defi.holds<DefModule>()) {
        return defi.as<DefModule>().scope;
    }
    auto maybe_structure = try_scope_for_top_level_def(did);
    if (maybe_structure.has_value()) {
        return maybe_structure.as_id();
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // recursive up to find parent's parent scope, etc.
    return containing_scope(maybe_parent.as_id());
}

} // namespace hir
