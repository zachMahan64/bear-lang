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
#include "compiler/hir/compt_expr_solver.hpp"
#include "compiler/hir/def.hpp"
#include "compiler/hir/def_visitor.hpp"
#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/file.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/scope.hpp"
#include "compiler/hir/span.hpp"
#include "compiler/hir/type.hpp"
#include "compiler/parser/token_eaters.h"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/data_arena.hpp"
#include "utils/log.hpp"
#include "llvm/ADT/SmallVector.h"
#include <atomic>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <iso646.h>
#include <optional>
#include <stddef.h>
#include <string_view>
namespace hir {

static constexpr size_t DEFAULT_SYMBOL_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_SCOPE_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_CANONICAL_TYPE_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_CANONICAL_GEN_ARGS_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_ID_MAP_ARENA_CAP
    = 0x8000; // increase if any other top level maps need to be mades
static constexpr size_t DEFAULT_TEMP_SCOPE_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_SYM_TO_FILE_ID_MAP_CAP = 0x80;
static constexpr size_t DEFAULT_SCOPE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_AST_VEC_CAP = DEFAULT_FILE_VEC_CAP;
static constexpr size_t DEFAULT_FILE_ID_VEC_CAP = 0x200;
static constexpr size_t DEFAULT_SYMBOL_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_EXEC_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_DEF_CAP = 0x800;
static constexpr size_t DEFAULT_TYPE_VEC_CAP = 0x400;
static constexpr size_t DEFAULT_CANONICAL_TYPE_VEC_CAP = 0x100;
static constexpr size_t DEFAULT_GENERIC_ARG_VEC_CAP = 0x400;
static constexpr HirSize EXPECTED_HIGH_NUM_IMPORTS = 128;
static constexpr size_t DEFAULT_DIAG_NUM = 0x100;
static constexpr size_t DEFAULT_DEF_SLICE_COUNT = 0x100;
static constexpr size_t DEFAULT_CANONICAL_TT_CAP = 0x400;
static constexpr size_t DEFAULT_CANONICAL_GEN_ARGS_CAP = 0x400;

Context::Context(const bearc_args_t& args) : Context(args, instances::multiple) {}

std::atomic<bool> Context::one_instance_status = true;

Context::Context(const bearc_args_t& args, instances instances)
    : file_ids{DEFAULT_FILE_ID_VEC_CAP}, files{DEFAULT_FILE_VEC_CAP},
      id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
      symbol_id_to_file_id_map{id_map_arena, DEFAULT_SYM_TO_FILE_ID_MAP_CAP},
      file_asts{DEFAULT_FILE_AST_VEC_CAP}, importer_to_importees{DEFAULT_FILE_VEC_CAP},
      importee_to_importers{DEFAULT_FILE_VEC_CAP}, file_to_diagnostics{EXPECTED_HIGH_NUM_IMPORTS},
      scope_arena{DEFAULT_SCOPE_ARENA_CAP}, scopes{DEFAULT_SCOPE_VEC_CAP},
      temp_scope_arena{std::make_unique<DataArena>(DEFAULT_TEMP_SCOPE_ARENA_CAP)},
      symbol_storage_arena{DEFAULT_SYMBOL_ARENA_CAP}, symbol_map_arena{DEFAULT_SYMBOL_ARENA_CAP},
      str_to_symbol_id_map{symbol_map_arena}, symbol_ids{DEFAULT_SYMBOL_VEC_CAP},
      symbols{DEFAULT_SYMBOL_VEC_CAP}, exec_ids{DEFAULT_EXEC_VEC_CAP}, execs{DEFAULT_EXEC_VEC_CAP},
      def_ids{DEFAULT_DEF_CAP}, defs{DEFAULT_DEF_CAP}, def_resol_states{DEFAULT_DEF_CAP},
      def_ast_nodes(DEFAULT_DEF_CAP), def_mention_states{DEFAULT_DEF_CAP},
      def_to_scope_for_types{id_map_arena, DEFAULT_DEF_CAP},
      def_to_scope_for_funcs{id_map_arena, DEFAULT_DEF_CAP}, ordered_def_slices{DEFAULT_DEF_CAP},
      def_to_ordered_def_slice_id{id_map_arena, DEFAULT_DEF_SLICE_COUNT}, type_ids{DEFAULT_DEF_CAP},
      types{DEFAULT_TYPE_VEC_CAP}, canonical_to_type_id(DEFAULT_CANONICAL_TYPE_VEC_CAP),
      canonical_type_table_arena{DEFAULT_CANONICAL_TYPE_ARENA_CAP},
      canonical_type_table(*this, canonical_type_table_arena, DEFAULT_CANONICAL_TT_CAP),
      generic_arg_ids{DEFAULT_GENERIC_ARG_VEC_CAP}, generic_args{DEFAULT_GENERIC_ARG_VEC_CAP},
      generic_args_arena{DEFAULT_CANONICAL_TYPE_ARENA_CAP},
      canonical_generic_args_id_to_def_id_map{DEFAULT_CANONICAL_GEN_ARGS_CAP},
      canonical_generic_args_to_first_instance{DEFAULT_CANONICAL_GEN_ARGS_CAP},
      generic_arg_id_slices{DEFAULT_CANONICAL_GEN_ARGS_CAP},
      canonical_generic_args_table_arena{DEFAULT_CANONICAL_GEN_ARGS_ARENA_CAP},
      canonical_compt_args_table{*this, canonical_generic_args_table_arena,
                                 DEFAULT_CANONICAL_GEN_ARGS_CAP},
      diagnostics{DEFAULT_DIAG_NUM}, diagnostics_used{DEFAULT_DIAG_NUM},
      only_one_context_instance((instances == instances::one) && one_instance_status), args{args},
      compact_diagnostics(args.flags[CLI_FLAG_COMPACT_DIAGS]) {

    one_instance_status = false; // we exist now

    // this may only fail in horribly malfored arguments in test cases
    assert(args.input_file_name);

    // get try to get root file, and allow checking cwd for it

    std::optional<std::filesystem::path> maybe_root_file
        = resolve_on_import_path(args.input_file_name, ".", &args);

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
        if (!has_flag(CLI_FLAG_PARSE_ONLY)) {
            FileAstVisitor visitor{*this, id};
            visitor.register_top_level_declarations();
        }
        this->note_cnt += ast.diagnostic_count() - ast.error_count();
        this->fatal_error_cnt += ast.error_count();
    }
    if (has_flag(CLI_FLAG_PARSE_ONLY)) {
        return;
    }
    TopLevelDefVisitor{*this}.resolve_top_level_definitions();
}

int Context::diagnostic_count() const noexcept {
    return static_cast<int>(fatal_error_cnt + normal_error_cnt + note_cnt + warning_cnt + help_cnt);
}

int Context::error_count() const noexcept {
    return static_cast<int>(this->fatal_error_cnt + this->normal_error_cnt);
}

int Context::warning_count() const noexcept { return static_cast<int>(warning_cnt); }
int Context::note_count() const noexcept { return static_cast<int>(note_cnt); }
int Context::help_count() const noexcept { return static_cast<int>(help_cnt); }

bool Context::compact_diagnostics_enabled() const noexcept { return compact_diagnostics; }

bool Context::has_flag(cli_flag_e flag) const noexcept { return args.flags[flag]; }

SymbolId Context::symbol_id(std::string_view sv) { return symbol_id(sv.data(), sv.length()); }
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
SymbolId Context::concat_symbols(SymbolId sid1, SymbolId sid2) {
    std::string buf{};
    buf.reserve(1024); // decently large
    buf += symbol_id_to_cstr(sid1);
    buf += symbol_id_to_cstr(sid2);
    return symbol_id(buf);
}
SymbolId Context::symbol_id_for_identifier_tkn(const token_t* tkn) {
    assert(token_is_builtin_type_or_id(tkn->type));
    return symbol_id(tkn->start, tkn->len);
}

SymbolId Context::symbol_id(Span span) { return symbol_id(span.as_sv(*this)); }
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
    // ****************** all lexing and parsing done in this one line
    FileAstId ast_id = this->file_asts.emplace_and_get_id(symbol_id_to_cstr(path_symbol));
    // ^^^^^^^^^^^^^^^^^^
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

std::string_view Context::symbol(SymbolId id) const { return this->symbols.cat(id).sv(); }

/**
 * updates the slice storing the importers for an importee
 */
void Context::register_importer(FileId importee, FileId importer) {
    importee_to_importers.at(importee).emplace_back(importer);
}

void Context::report_cycle(llvm::SmallVectorImpl<FileId>& import_stack,
                           const token_t* import_path_tkn) {
    // gonna be imported in the previous thing, so top of import stack
    FileId imported_in = import_stack[import_stack.size() - 1];
    emplace_diagnostic(Span{imported_in, ast(imported_in).buffer(), import_path_tkn},
                       diag_code::cyclical_import, diag_type::warning,
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
        report_cycle(import_stack, import_path_tkn);
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
    if (has_flag(CLI_FLAG_FILE_GRAPH)) {
        std::cout << ansi_bold_reset() << "all files" << '(' << files.size() << ')' << ":"
                  << ansi_reset() << '\n';
        for (FileId curr = files.begin_id(); curr != files.end_id(); ++curr) {

            const FileAst& ast = file_asts.cat(files.cat(curr).ast_id);

            std::cout << ansi_bold_reset() << '[' << curr.val() << "] " << ast.file_name();
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
    if (!has_flag(CLI_FLAG_SILENT)) {
        // go thru each file ast to print info
        for (auto fid = files.begin_id(); fid != files.end_id(); fid++) {
            const FileAst& aast = ast(fid);
            // 1. print parse-time errors (ast-wise errors)
            aast.print_all_errors(compact_diagnostics_enabled());
            // 2. print diagnostics (semantic/non-grammatical errors)
            // OptId<DiagnosticId> prev_diag{};
            for (const auto d : file_to_diagnostics.cat(fid)) {
                /*
                bool print_file = true;
                // check if we should bother printing the file name again
                if (prev_diag.has_value()) {
                    auto curr = diagnostics.cat(d);
                    auto prev = diagnostics.cat(prev_diag.as_id());
                    print_file = curr.span.file_id != prev.span.file_id;
                }
                */
                print_diagnostic(d /*, print_file*/);
                // rotate
                // prev_diag = d;
            }
        }
    }
    if (!has_flag(CLI_FLAG_SILENT)) {
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
        auto helps = help_count();
        if (helps == 1) {
            puts("1 tip generated.");
        } else if (helps != 0) {
            printf("%d tips generated.\n", helps);
        }
    }
    // std::cout << tables.files.size() << '\n';
    if (this->diagnostic_count() != 0) {
        if (!has_flag(CLI_FLAG_SILENT)) {
            printf("compilation terminated: %s'%s'\n%s", ansi_bold_reset(),
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
    const token_t* path_tkn = import_statement->stmt.import.file_path;
    SymbolId path_symbol_id = symbol_id_for_str_lit_tkn(path_tkn);
    const char* path = symbol_id_to_cstr(path_symbol_id);

    const std::filesystem::path parent = std::filesystem::path(path).parent_path();

    // DNE guard
    auto maybe_path = resolve_on_import_path(path, parent, &this->args);
    if (!maybe_path.has_value()) {
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

DefId Context::register_compt_param(SymbolId name, Span span, DefId parent, DefValue value) {
    DefId def = defs.emplace_and_get_id(value, name, true, true, true, false, span, parent);
    def_resol_states.bump(Def::resol_state::resolved);
    def_ast_nodes.bump();
    def_mention_states.bump(Def::mention_state::unmentioned);
    return def;
}

void Context::insert_variable(ScopeId scope_id, SymbolId sid, DefId did) {
    scopes.at(scope_id).insert_variable(sid, did);
}

[[nodiscard]] IdHashMap<DefId, ScopeId>& Context::defs_to_scopes_for_types() {
    return def_to_scope_for_types;
}

[[nodiscard]] DefId Context::begin_def_id() const { return defs.begin_id(); }
[[nodiscard]] DefId Context::end_def_id() const { return defs.end_id(); }

ExecId Context::emplace_exec(const ExecValue& value, Span span, bool should_be_compt) {
    return execs.emplace_and_get_id(*this, value, span, should_be_compt);
}

ExecId Context::emplace_compt_exec(const ExecValue& value, Span span) {
    return execs.emplace_and_get_id(*this, value, span, true);
}

FileAst& Context::ast(FileId file_id) { return file_asts.at(files.at(file_id).ast_id); }

const FileAst& Context::ast(FileId file_id) const {
    return file_asts.cat(files.cat(file_id).ast_id);
}

ScopeId Context::get_or_make_root_scope() {
    if (scopes.size() == 0) {
        return make_scope(std::nullopt);
    }
    // the top-level scope will have to be the first scope!
    return ScopeId{1};
}

ScopeId Context::root_scope() const {
    // the top-level scope will have to be the first scope!
    assert(this->scopes.size() != 0 && "tried to get the root scope before its creation");
    return ScopeId{1};
}

ScopeId Context::make_scope(OptId<ScopeId> parent_scope) {
    return scopes.emplace_and_get_id(parent_scope, scope_arena);
}

ScopeId Context::make_small_scope(OptId<ScopeId> parent_scope) {
    static constexpr size_t CAP = 0x8;
    return make_scope(parent_scope, CAP);
}

ScopeId Context::make_medium_scope(OptId<ScopeId> parent_scope) {
    static constexpr size_t CAP = 0x10;
    return make_scope(parent_scope, CAP);
}

ScopeId Context::make_scope(OptId<ScopeId> parent_scope, HirSize capacity) {
    return scopes.emplace_and_get_id(parent_scope, capacity, scope_arena);
}

ScopeId Context::make_compt_func_temp_scope(ScopeId parent_scope, HirSize capacity) {
    return scopes.emplace_and_get_id(parent_scope, capacity, *temp_scope_arena,
                                     Scope::storage::variables);
}

DefId Context::register_generated_deftype(ScopeId scope, SymbolId name, TypeId type_id,
                                          DefId parent, Span span) {
    auto did = defs.emplace_and_get_id(DefDeftype{.type = type_id}, name, false, false, false,
                                       false, span, parent);
    this->scope(scope).insert_type(name, did);
    def_resol_states.bump(Def::resol_state::resolved);
    def_ast_nodes.bump();
    def_mention_states.bump(Def::mention_state::unmentioned);
    return did;
}

bool Context::relinquish_temp_scopes() {

    const size_t chunk_size = temp_scope_arena->first_chunk_size();
    const size_t chunk_cap = temp_scope_arena->chunk_cap();

    // try to catch the arena before it has to alloc a second chunk, but don't do it too undersized.
    // estimate the value with 1/2 + 1/4 + 1/8 + 1/16 = 15/16
    if (chunk_size > ((chunk_cap >> 1) + (chunk_cap >> 2) + (chunk_cap >> 3) + (chunk_cap >> 4))) {
        temp_scope_arena
            = std::make_unique<DataArena>(chunk_cap); // frees old arena and gives us another
        return true;
    }
    return false;
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
    } else if (type == diag_type::help) {
        help_cnt++;
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
                                         DiagnosticInfoValue value, OptId<DiagnosticId> next) {
    DiagnosticId id = diagnostics.emplace_and_get_id(span, code, type, value, next);
    diagnostics_used.bump();
    file_to_diagnostics.at(span.file_id).emplace_back(id);
    handle_bump_diag_counts(code, type);
    return id;
}

DiagnosticId Context::emplace_diagnostic(Span span, diag_code code, diag_type type,
                                         DiagnosticMessageValue message_value,
                                         DiagnosticInfoValue value, OptId<DiagnosticId> next) {
    DiagnosticId id = diagnostics.emplace_and_get_id(span, code, type, message_value, value, next);
    diagnostics_used.bump();
    file_to_diagnostics.at(span.file_id).emplace_back(id);
    handle_bump_diag_counts(code, type);
    return id;
}

DiagnosticId Context::emplace_diagnostic_with_message_value(Span span, diag_code code,
                                                            diag_type type,
                                                            DiagnosticMessageValue message_value,
                                                            OptId<DiagnosticId> next) {
    DiagnosticId id = diagnostics.emplace_and_get_id(span, code, type, message_value,
                                                     DiagnosticNoOtherInfo{}, next);
    diagnostics_used.bump();
    if (!span.is_generated()) {
        file_to_diagnostics.at(span.file_id).emplace_back(id);
    }
    handle_bump_diag_counts(code, type);
    return id;
}

void Context::print_diagnostic(DiagnosticId diag_id, bool print_file) {
    // as to not re-report
    if (diagnostics_used.cat(diag_id)) {
        return;
    }
    const Diagnostic& diag = diagnostics.cat(diag_id);
    diag.print(*this, print_file);
    // mark used
    diagnostics_used.at(diag_id) = true;
    // try print next
    if (diag.next.has_value() && !diagnostics_used.cat(diag.next.as_id())) {
        // only print next's file if it differs
        Span next_span = diagnostics.cat(diag.next.as_id()).span;
        bool print_next_file
            = !next_span.is_generated() && (diag.span.file_id != next_span.file_id);
        print_diagnostic(diag.next.as_id(), print_next_file);
    } else if (!compact_diagnostics_enabled()) {
        std::cout << '\n'; // this makes it so there's a new line between the start and end of
                           // none-contiguous diagnostics, which is more readable
    }
}

void Context::link_diagnostic(DiagnosticId diag, DiagnosticId next) {
    diagnostics.at(diag).set_next(next);
}

Def& Context::def(DefId def_id) { return defs.at(def_id); }

const Def& Context::try_func_def(DefId def_id) const { return def(try_func_did(def_id)); }

DefId Context::try_func_did(DefId def_id) const {
    const Def& def = this->def(def_id);
    if (def.holds<DefVariable>()) {
        DefVariable var = def.as<DefVariable>();
        if (var.compt_value.has_value()) {
            ExecId compt_val = var.compt_value.as_id();
            const Exec& compt_exec = exec(compt_val);
            if (compt_exec.holds<ExecFnPtr>()) {
                return compt_exec.as<ExecFnPtr>().func_def_id;
            }
        }
    }
    return def_id;
}

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

Def::mention_state Context::mention_state_of(DefId def) const {
    return def_mention_states.cat(def);
}
void Context::promote_mention_state_of(DefId def, Def::mention_state new_mention_state) {
    // only promote (when current is less than new)
    if (mention_state_of(def) < new_mention_state) {
        def_mention_states.at(def) = new_mention_state;
    }
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

[[nodiscard]] const ast_stmt_t* Context::def_ast_node(DefId def_id) const {
    return def_ast_nodes.cat(def_id);
}

[[nodiscard]] bool Context::is_struct_def(DefId def_id) const {
    return def_ast_nodes.cat(def_id)->type == AST_STMT_STRUCT_DEF;
}

OptId<ScopeId> Context::try_scope_for_top_level_def(DefId def_id) const {
    const auto& def = defs.cat(def_id);
    // no parent means parent scope is root scope
    if (def.holds<DefModule>()) {
        return def.as<DefModule>().scope;
    }
    if (def.holds<DefDeftype>()) {
        const Type& type = this->type(try_decay_ref(def.as<DefDeftype>().type));
        if (type.holds<TypeStructure>()) {
            return def_to_scope_for_types.at(type.as<TypeStructure>().definition);
        }
    }
    // hopefully found
    return def_to_scope_for_types.at(def_id);
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

const Type& Context::type(IdIdx<TypeId> ididx) const { return type(type_ids.cat(ididx)); }
Type& Context::type(IdIdx<TypeId> ididx) { return type(type_ids.at(ididx)); }
const Type& Context::type(TypeId id) const {
    const Type* t = &types.cat(id);
    while (t->holds<TypeDeftype>()) {
        t = &types.cat(t->as<TypeDeftype>().true_type);
    }
    return *t;
}
Type& Context::type(TypeId id) {
    Type* t = &types.at(id);
    while (t->holds<TypeDeftype>()) {
        t = &types.at(t->as<TypeDeftype>().true_type);
    }
    return *t;
}

TypeId Context::try_decay_ref(TypeId tid) const {
    const Type& type = this->type(tid);
    if (type.holds<TypeRef>()) {
        return type.as<TypeRef>().inner;
    }
    return tid;
}

/// gets the type value without bypassing deftypes
const Type& Context::type_as_mentioned(IdIdx<TypeId> ididx) const {
    return type_as_mentioned(type_ids.cat(ididx));
}
/// gets the type value without bypassing deftypes
Type& Context::type_as_mentioned(IdIdx<TypeId> ididx) {
    return type_as_mentioned(type_ids.at(ididx));
}

[[nodiscard]] const Type& Context::type_as_mentioned(TypeId id) const { return types.cat(id); }
/// gets the type value without bypassing deftypes
[[nodiscard]] Type& Context::type_as_mentioned(TypeId id) { return types.at(id); }
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

ExecId Context::exec_id(IdIdx<ExecId> id) const { return exec_ids.cat(id); }

[[nodiscard]] const Scope& Context::scope(ScopeId sid) const { return scopes.cat(sid); }

[[nodiscard]] const Def& Context::def(IdIdx<DefId> id) const { return defs.cat(def_ids.cat(id)); }

[[nodiscard]] OptId<DefId> Context::try_struct_def(DefId did) const {
    const Def& d = def(did);

    if (d.holds<DefStruct>()) {
        return did;
    }

    if (d.holds<DefDeftype>()) {
        const Type& t = type(d.as<DefDeftype>().type);
        if (t.holds<TypeStructure>()) {
            return t.as<TypeStructure>().definition;
        }
    }

    return std::nullopt;
}

[[nodiscard]] DefId Context::def_id(IdIdx<DefId> id) const { return def_ids.cat(id); }

OptId<DefId> Context::look_up_variable(ScopeId scope, SymbolId sid) const {
    return Scope::look_up_variable(*this, scope, sid);
}
OptId<DefId> Context::look_up_type(ScopeId scope, SymbolId sid) const {
    return Scope::look_up_type(*this, scope, sid);
}
OptId<DefId> Context::look_up_namespace(ScopeId scope, SymbolId sid) const {
    return Scope::look_up_namespace(*this, scope, sid);
}

OptId<DefId> Context::look_up_scoped(auto F, ScopeId scope, IdSlice<SymbolId> id_slice,
                                     Span id_span) {
    ScopeId curr_scope = scope;
    for (IdIdx<SymbolId> sidx = id_slice.begin(); sidx != id_slice.end(); sidx++) {
        SymbolId sid = symbol_ids.cat(sidx);
        // base case, last elem should be the variable
        if (sidx == id_slice.last_elem()) {
            OptId<DefId> maybe = F(curr_scope, sid);
            return maybe.has_value() ? guard_hid(F, scope, maybe.as_id(), id_slice, id_span)
                                     : maybe;
        }
        if (auto maybe_mod = look_up_namespace(curr_scope, sid); maybe_mod.has_value()) {
            curr_scope = def(guard_hid_namespace(
                                 scope, maybe_mod.as_id(),
                                 IdSlice<SymbolId>{id_slice.begin(),
                                                   sidx.val() + 1 - id_slice.begin().val()},
                                 id_span))
                             .as<DefModule>()
                             .scope;
        } else if (auto maybe_type = look_up_type(curr_scope, sid); maybe_type.has_value()) {
            curr_scope = scope_for_top_level_def(guard_hid_type(
                scope, maybe_type.as_id(),
                IdSlice<SymbolId>{id_slice.begin(), sidx.val() + 1 - id_slice.begin().val()},
                id_span));
        }
    }
    // never entered the loop, so not found
    return OptId<DefId>{};
}

OptId<DefId> Context::look_up_scoped_variable(ScopeId scope, IdSlice<SymbolId> id_slice,
                                              Span id_span) {
    return look_up_scoped(
        [this](ScopeId scope, SymbolId sid) { return look_up_variable(scope, sid); }, scope,
        id_slice, id_span);
}

OptId<DefId> Context::look_up_scoped_type(ScopeId scope, IdSlice<SymbolId> id_slice, Span id_span) {
    return look_up_scoped([this](ScopeId scope, SymbolId sid) { return look_up_type(scope, sid); },
                          scope, id_slice, id_span);
}
OptId<DefId> Context::look_up_scoped_namespace(ScopeId scope, IdSlice<SymbolId> id_slice,
                                               Span id_span) {
    return look_up_scoped(
        [this](ScopeId scope, SymbolId sid) { return look_up_namespace(scope, sid); }, scope,
        id_slice, id_span);
}
DefId Context::guard_hid(auto F, ScopeId scope, DefId did, IdSlice<SymbolId> id_slice,
                         Span id_span) {
    const Def& defin = this->def(did);
    if (defin.pub) {
        return did;
    }

    const OptId<DefId> maybe_locally_availible = F(scope, defin.name);
    if (!maybe_locally_availible.has_value() || maybe_locally_availible.as_id() != did) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::is_declared_hid, diag_type::error,
            DiagnosticIdentifierBeforeMessage{.sid_slice = id_slice});

        auto d1 = emplace_diagnostic_with_message_value(
            defin.span, diag_code::declared_here, diag_type::note,
            DiagnosticIdentifierBeforeMessage{.sid_slice = id_slice});

        link_diagnostic(d0, d1);
    }
    return did;
}
DefId Context::guard_hid_type(ScopeId scope, DefId did, IdSlice<SymbolId> id_slice, Span id_span) {
    return guard_hid([this](ScopeId scope, SymbolId sid) { return look_up_type(scope, sid); },
                     scope, did, id_slice, id_span);
}

DefId Context::guard_hid_variable(ScopeId scope, DefId did, IdSlice<SymbolId> id_slice,
                                  Span id_span) {
    return guard_hid([this](ScopeId scope, SymbolId sid) { return look_up_variable(scope, sid); },
                     scope, did, id_slice, id_span);
}

DefId Context::guard_hid_namespace(ScopeId scope, DefId did, IdSlice<SymbolId> id_slice,
                                   Span id_span) {
    return guard_hid([this](ScopeId scope, SymbolId sid) { return look_up_namespace(scope, sid); },
                     scope, did, id_slice, id_span);
}

OptId<DefId> Context::look_up_scoped_bypassing_visibility(auto F, ScopeId scope,
                                                          IdSlice<SymbolId> id_slice) {
    ScopeId curr_scope = scope;
    for (IdIdx<SymbolId> sidx = id_slice.begin(); sidx != id_slice.end(); sidx++) {
        SymbolId sid = symbol_ids.cat(sidx);
        // base case, last elem should be the variable
        if (sidx == id_slice.last_elem()) {
            OptId<DefId> maybe = F(curr_scope, sid);
            return maybe;
        }
        if (auto maybe_mod = look_up_namespace(curr_scope, sid); maybe_mod.has_value()) {
            curr_scope = def(maybe_mod.as_id()).as<DefModule>().scope;
        } else if (auto maybe_type = look_up_type(curr_scope, sid); maybe_type.has_value()) {
            curr_scope = scope_for_top_level_def(maybe_type.as_id());
        }
    }
    // not found fallback
    return OptId<DefId>{};
}
OptId<DefId> Context::look_up_scoped_variable_bypassing_visibility(ScopeId scope,
                                                                   IdSlice<SymbolId> id_slice) {
    return look_up_scoped_bypassing_visibility(
        [this](ScopeId scope, SymbolId sid) { return look_up_variable(scope, sid); }, scope,
        id_slice);
}

OptId<DefId> Context::look_up_scoped_type_bypassing_visibility(ScopeId scope,
                                                               IdSlice<SymbolId> id_slice) {
    return look_up_scoped_bypassing_visibility(
        [this](ScopeId scope, SymbolId sid) { return look_up_type(scope, sid); }, scope, id_slice);
}
OptId<DefId> Context::look_up_scoped_namespace_bypassing_visibility(ScopeId scope,
                                                                    IdSlice<SymbolId> id_slice) {
    return look_up_scoped_bypassing_visibility(
        [this](ScopeId scope, SymbolId sid) { return look_up_namespace(scope, sid); }, scope,
        id_slice);
}

OptId<DefId> Context::look_up_member_var_guarding_hid(const Def& struct_def, SymbolId symbol_id,
                                                      Span id_span, ScopeId local_scope) {
    auto maybe_def
        = Scope::look_up_local_variable(*this, struct_def.as<DefStruct>().scope, symbol_id);
    if (maybe_def.empty()) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::id_does_not_name_a_member_variable_of, diag_type::error,
            DiagnosticSymbolAfterMessage{.sid = struct_def.name});
        auto d1 = emplace_diagnostic_with_message_value(
            struct_def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = struct_def.name});
        link_diagnostic(d0, d1);
        return std::nullopt;
    }
    const Def& def = this->def(maybe_def.as_id());
    if (!def.holds<DefVariable>()) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::id_does_not_name_a_member_variable_of, diag_type::error,
            DiagnosticSymbolAfterMessage{.sid = struct_def.name});
        auto d1 = emplace_diagnostic_with_message_value(
            def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = def.name});
        link_diagnostic(d0, d1);
        return std::nullopt;
    }
    if (!def.is_ordered()) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::id_names_a_static_mem_thru_dot_for, diag_type::error,
            DiagnosticSymbolAfterMessage{.sid = struct_def.name});
        auto d1 = emplace_diagnostic_with_message_value(
            def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = def.name});
        link_diagnostic(d0, d1);
        return std::nullopt;
    }
    if (!def.pub && !scope_has_parent(local_scope, struct_def.as<DefStruct>().scope)) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::is_declared_hid, diag_type::error,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        auto d1 = emplace_diagnostic_with_message_value(
            def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        link_diagnostic(d0, d1);
    }
    return maybe_def;
}

OptId<DefId> Context::look_up_member_function_guarding_hid(const Def& struct_def,
                                                           SymbolId symbol_id, Span id_span,
                                                           ScopeId local_scope) {
    auto maybe_def
        = Scope::look_up_local_variable(*this, struct_def.as<DefStruct>().scope, symbol_id);
    if (maybe_def.empty()) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::id_does_not_name_a_method_of, diag_type::error,
            DiagnosticSymbolAfterMessage{.sid = struct_def.name});
        auto d1 = emplace_diagnostic_with_message_value(
            struct_def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = struct_def.name});
        link_diagnostic(d0, d1);
        return std::nullopt;
    }
    const Def& def = this->try_func_def(maybe_def.as_id());
    if (!def.holds<DefFunction>()) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::id_does_not_name_a_method_of, diag_type::error,
            DiagnosticSymbolAfterMessage{.sid = struct_def.name});
        auto d1 = emplace_diagnostic_with_message_value(
            def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = def.name});
        link_diagnostic(d0, d1);
        return std::nullopt;
    }
    if (!def.pub && !scope_has_parent(local_scope, struct_def.as<DefStruct>().scope)) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::is_declared_hid, diag_type::error,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        auto d1 = emplace_diagnostic_with_message_value(
            def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        link_diagnostic(d0, d1);
    }
    return maybe_def;
}

OptId<DefId> Context::look_up_member_function_no_diag_except_hid(const Def& struct_def,
                                                                 SymbolId symbol_id, Span id_span,
                                                                 ScopeId local_scope) {
    auto maybe_def
        = Scope::look_up_local_variable(*this, struct_def.as<DefStruct>().scope, symbol_id);
    if (maybe_def.empty()) {
        return std::nullopt;
    }
    const Def& def = this->try_func_def(maybe_def.as_id());
    if (!def.holds<DefFunction>()) {
        return std::nullopt;
    }
    if (!def.pub && !scope_has_parent(local_scope, struct_def.as<DefStruct>().scope)) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::is_declared_hid, diag_type::error,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        auto d1 = emplace_diagnostic_with_message_value(
            def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        link_diagnostic(d0, d1);
    }
    return maybe_def;
}

OptId<DefId> Context::look_up_member_var_no_diag_except_hid(const Def& struct_def,
                                                            SymbolId symbol_id, Span id_span,
                                                            ScopeId local_scope) {
    auto maybe_def
        = Scope::look_up_local_variable(*this, struct_def.as<DefStruct>().scope, symbol_id);
    if (maybe_def.empty()) {
        return std::nullopt;
    }
    const Def& def = this->def(maybe_def.as_id());
    if (!def.holds<DefVariable>()) {
        return std::nullopt;
    }
    if (!def.pub && !scope_has_parent(local_scope, struct_def.as<DefStruct>().scope)) {
        auto d0 = emplace_diagnostic_with_message_value(
            id_span, diag_code::is_declared_hid, diag_type::error,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        auto d1 = emplace_diagnostic_with_message_value(
            def.span, diag_code::declared_here, diag_type::note,
            DiagnosticSymbolBeforeMessage{.sid = symbol_id});
        link_diagnostic(d0, d1);
    }
    return maybe_def;
}

bool Context::scope_has_parent(ScopeId local_scope, ScopeId possible_parent) const {
    ScopeId curr_scope = local_scope;
    while (true) {
        if (curr_scope == possible_parent) {
            return true;
        }
        const Scope* curr = &scope(curr_scope);

        if (curr->parent().empty()) {
            break;
        }
        curr_scope = curr->parent().as_id();
    }
    return false;
}

[[nodiscard]] bool Context::defined_bypassing_visibility(ScopeId scope,
                                                         IdSlice<SymbolId> id_slice) {
    auto maybe_type = look_up_scoped_type_bypassing_visibility(scope, id_slice);
    if (maybe_type.has_value()) {
        return true;
    }
    return look_up_scoped_variable_bypassing_visibility(scope, id_slice).has_value();
}

[[nodiscard]] bool Context::defined(ScopeId scope, IdSlice<SymbolId> id_slice, Span id_span,
                                    bool member) {

    if (member) {
        OptId<DefId> curr_struct;
        for (HirSize i = 0; i < id_slice.len(); i++) {
            SymbolId sid = symbol_id(id_slice.get(i));
            OptId<DefId> maybe_did;
            if (curr_struct.empty()) {
                maybe_did = look_up_variable(scope, sid);
            } else {
                maybe_did = look_up_member_function_no_diag_except_hid(def(curr_struct.as_id()),
                                                                       sid, id_span, scope);
                if (maybe_did.empty()) {
                    maybe_did = look_up_member_var_no_diag_except_hid(def(curr_struct.as_id()), sid,
                                                                      id_span, scope);
                }
            }

            if (maybe_did.has_value() && i == id_slice.len() - 1) {
                return true;
            }

            if (maybe_did.empty()) {
                return false;
            }
            auto did = maybe_did.as_id();
            const Def& def = this->def(did);
            // try get chained structs
            if (def.holds<DefVariable>()) {
                const DefVariable var_def = def.as<DefVariable>();
                const Type& type = this->type(try_decay_ref(def.as<DefVariable>().type));
                if (type.holds<TypeStructure>()) {
                    curr_struct = type.as<TypeStructure>().definition;
                } else if (var_def.compt_value.has_value()) {
                    TopLevelDefVisitor def_vis{*this};
                    ComptExprSolver<TopLevelDefVisitor> solver{*this, def_vis};
                    const OptId<TypeId> maybe_tid
                        = solver.infer_type_from_compt_exec(var_def.compt_value.as_id());
                    if (maybe_tid.has_value()
                        && this->type(maybe_tid.as_id()).holds<TypeStructure>()) {
                        curr_struct = this->type(maybe_tid.as_id()).as<TypeStructure>().definition;
                    }
                }
            }
        }
        return true;
    }

    // bypass vis the second time around to not warn multiple times
    auto maybe_type = look_up_scoped_type(scope, id_slice, id_span);

    auto err_when_hid = [this, id_slice, id_span](DefId did) {
        const Def& deffy = def(did);
        if (!deffy.pub) {
            auto d0 = emplace_diagnostic_with_message_value(
                id_span, diag_code::is_declared_hid, diag_type::error,
                DiagnosticIdentifierBeforeMessage{.sid_slice = id_slice});
            auto d1 = emplace_diagnostic_with_message_value(
                deffy.span, diag_code::declared_here, diag_type::note,
                DiagnosticIdentifierBeforeMessage{.sid_slice = id_slice});
            link_diagnostic(d0, d1);
        }
    };

    if (maybe_type.has_value()) {
        err_when_hid(maybe_type.as_id());
        return true;
    }
    auto maybe_var = look_up_scoped_variable_bypassing_visibility(scope, id_slice);

    if (maybe_var.has_value()) {
        err_when_hid(maybe_var.as_id());
        return true;
    }
    return false;
}

IdSlice<SymbolId> Context::symbol_slice(token_ptr_slice_t token_slice) {
    llvm::SmallVector<SymbolId> vec{};
    for (size_t i = 0; i < token_slice.len; i++) {
        const token_t* tkn = token_slice.start[i];
        vec.push_back(symbol_id(tkn));
    }
    return freeze_id_vec(vec);
}

ScopeId Context::containing_scope(DefId did) const {
    auto maybe_parent = defs.cat(did).parent;
    // ----- base cases (parent w/ scope)---------
    if (!maybe_parent.has_value()) {
        return root_scope();
    }
    DefId parent_id = maybe_parent.as_id();
    const Def& par_def = def(parent_id);
    if (par_def.holds<DefModule>()) {
        return par_def.as<DefModule>().scope;
    }
    if (par_def.holds<DefScopeWrapper>()) {
        return par_def.as<DefScopeWrapper>().scope;
    }
    auto maybe_structure = try_scope_for_top_level_def(parent_id);
    if (maybe_structure.has_value()) {
        return maybe_structure.as_id();
    }
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
    // recursive up to find parent's parent scope, etc.
    return containing_scope(parent_id);
}

void Context::register_func_to_scope(DefId did, ScopeId scope_id) {
    def_to_scope_for_funcs.insert(did, scope_id);
}

OptId<ScopeId> Context::func_to_scope(DefId did) { return def_to_scope_for_funcs.at(did); }

SymbolId Context::symbol_id(IdIdx<SymbolId> sididx) const { return symbol_ids.cat(sididx); }

[[nodiscard]] bool Context::equivalent_type(TypeId tid1, TypeId tid2) const {
    return type(tid1).canonical == type(tid2).canonical;
}

} // namespace hir
