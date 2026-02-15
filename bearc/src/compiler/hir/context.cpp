//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/context.hpp"
#include "cli/args.h"
#include "cli/import_path.h"
#include "compiler/ast/printer.h"
#include "compiler/ast/stmt.h"
#include "compiler/hir/file.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/log.hpp"
#include "llvm/ADT/SmallVector.h"
#include <atomic>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stddef.h>
#include <string_view>
namespace hir {

static constexpr size_t DEFAULT_SYMBOL_ARENA_CAP = 0x10000;
static constexpr size_t DEFAULT_ID_MAP_ARENA_CAP
    = 0x1000; // increase if any other top level maps need to be made
static constexpr size_t DEFAULT_SYM_TO_FILE_ID_MAP_CAP = 0x80;
static constexpr size_t DEFAULT_SCOPE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_FILE_AST_VEC_CAP = DEFAULT_FILE_VEC_CAP;
static constexpr size_t DEFAULT_FILE_ID_VEC_CAP = 0x200;
static constexpr size_t DEFAULT_SCOPE_ANON_VEC_CAP = 0x100;
static constexpr size_t DEFAULT_SYMBOL_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_EXEC_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_DEF_VEC_CAP = 0x800;
static constexpr size_t DEFAULT_TYPE_VEC_CAP = 0x400;
static constexpr size_t DEFAULT_GENERIC_PARAM_VEC_CAP = 0x80;
static constexpr size_t DEFAULT_GENERIC_ARG_VEC_CAP = 0x400;
static constexpr HirSize EXPECTED_HIGH_NUM_IMPORTS = 128;

Context::Context(const bearc_args_t* args)
    : symbol_storage_arena{DEFAULT_SYMBOL_ARENA_CAP}, id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
      symbol_id_to_file_id_map{id_map_arena, DEFAULT_SYM_TO_FILE_ID_MAP_CAP},
      scopes{DEFAULT_SCOPE_VEC_CAP}, files{DEFAULT_FILE_VEC_CAP},
      file_asts{DEFAULT_FILE_AST_VEC_CAP}, scope_anons{DEFAULT_SCOPE_ANON_VEC_CAP},
      symbols{DEFAULT_SYMBOL_VEC_CAP}, execs{DEFAULT_EXEC_VEC_CAP}, defs{DEFAULT_DEF_VEC_CAP},
      file_ids{DEFAULT_FILE_ID_VEC_CAP}, importer_to_importees{DEFAULT_FILE_VEC_CAP},
      importee_to_importers{DEFAULT_FILE_VEC_CAP}, symbol_ids{DEFAULT_SYMBOL_VEC_CAP},
      exec_ids{DEFAULT_EXEC_VEC_CAP}, def_ids{DEFAULT_DEF_VEC_CAP},
      def_resol_states{DEFAULT_DEF_VEC_CAP}, def_mention_states{DEFAULT_DEF_VEC_CAP},
      type_vec{DEFAULT_TYPE_VEC_CAP}, type_ids{DEFAULT_DEF_VEC_CAP},
      generic_param_ids{DEFAULT_GENERIC_PARAM_VEC_CAP},
      generic_params{DEFAULT_GENERIC_PARAM_VEC_CAP}, generic_arg_ids{DEFAULT_GENERIC_ARG_VEC_CAP},
      generic_args{DEFAULT_GENERIC_ARG_VEC_CAP}, symbol_map_arena{DEFAULT_SYMBOL_ARENA_CAP},
      str_to_symbol_id_map{symbol_map_arena}, args{args} {

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
    for (const auto f : files) {
        const FileAst& ast = file_asts.cat(f.ast_id);
        this->bump_parser_error_count(ast.error_count());
    }
}

void Context::bump_parser_error_count(uint32_t cnt) noexcept {
    parse_error_count.fetch_add(cnt, std::memory_order_relaxed);
}

int Context::error_count() const noexcept {
    return static_cast<int>(this->parse_error_count + this->semantic_error_count);
}

SymbolId Context::get_symbol_id(std::string_view str) {
    return get_symbol_id(str.data(), str.length());
}

SymbolId Context::get_symbol_id(const char* start, size_t len) {
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
SymbolId Context::get_symbol_id_for_tkn(token_t* tkn) {
    assert(tkn->type == TOK_IDENTIFIER);
    return get_symbol_id(tkn->start, tkn->len);
}
SymbolId Context::get_symbol_id_for_str_lit_token(token_t* tkn) {
    assert(tkn->type == TOK_STR_LIT);
    return get_symbol_id(tkn->start + 1, tkn->len - 2); // trims outer quotes
}

FileId Context::provide_root_file(const char* file_name) {
    SymbolId path_symbol = get_symbol_id(file_name, strlen(file_name));
    FileId file_id = get_file(path_symbol);
    return file_id;
}

FileId Context::get_file(SymbolId path_symbol) {
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
    return file_id;
}

FileId Context::get_file(std::filesystem::path& path) {
    SymbolId symbol_id = get_symbol_id(path.c_str());
    return get_file(symbol_id);
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

// TODO make this a tracked internal file error system (DNE, and circularity tracking)
void Context::report_cycle(FileId cyclical_file_id,
                           llvm::SmallVectorImpl<FileId>& import_stack) const {
    std::cout << ansi_bold_red() << "error" << ansi_reset()
              << ": circular file import: " << ansi_bold_white() << file_name(cyclical_file_id)
              << ansi_reset() << '\n';
    auto print_trace = [&](FileId id) {
        std::cout << "|    imported in: " << ansi_bold_white() << file_name(id) << ansi_reset();
    };
    for (auto it = import_stack.rbegin(); it != import_stack.rend(); ++it) {
        print_trace(*it);
        if (*it == cyclical_file_id) {
            std::cout << ansi_bold_white() << " <-- cycle origin\n";
            break;
        }
        std::cout << '\n';
    }
}
void Context::explore_imports(FileId root_id) {
    llvm::SmallVector<FileId> import_stack{};
    import_stack.push_back(root_id);
    explore_imports(root_id, import_stack);
}

void Context::explore_imports(FileId importer_file_id,
                              llvm::SmallVectorImpl<FileId>& import_stack) {
    auto& file = files.at(importer_file_id);

    // angry base case, guard circularity
    if (file.load_state == file_import_state::in_progress) {
        // safe to take id because a circularity is only possible if a prev id exists
        report_cycle(importer_file_id, import_stack);
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
            this->explore_imports(importee_file_id, import_stack);
            import_stack.pop_back();
        }
    }

    // set forward dependency list
    auto importer_to_importees_slice = file_ids.freeze_small_vec(importees);

    importer_to_importees.at(importer_file_id) = importer_to_importees_slice;

    file.load_state = file_import_state::done;
}

void Context::try_print_info() const {
    // go thru each file ast to print info
    for (const auto f : files) {
        const FileAst& ast = file_asts.cat(f.ast_id);
        ast.try_print_info(args);
    }
    if (args->flags[CLI_FLAG_LIST_FILES]) {
        std::cout << ansi_bold_white() << "all files" << '(' << files.size() << ')' << ":"
                  << ansi_reset() << '\n';
        for (FileId curr = files.first_id(); curr != files.last_id(); ++curr) {

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

    // std::cout << tables.files.size() << '\n';
    if (this->error_count() != 0) {
        if (!args->flags[CLI_FLAG_SILENT]) {
            printf("compilation terminated: %s'%s'\n%s", ansi_bold_white(),
                   symbol_id_to_cstr(files.cat(FileId{1}).path), ansi_reset());
        }
    }
    // release printer's internal state
    pretty_printer_reset();
}

const char* Context::file_name(FileId id) const { return symbol_id_to_cstr(files.cat(id).path); }

void Context::register_tokenwise_error(FileId file_id, token_t* tkn, error_code_e error_code) {
    file_asts.at(files.at(file_id).ast_id).emplace_tokenwise_error(tkn, error_code);
}

OptId<FileId> Context::try_file_from_import_statement(FileId importer_id,
                                                      const ast_stmt_t* import_statement) {
    assert(import_statement->type == AST_STMT_IMPORT);
    token_t* path_tkn = import_statement->stmt.import.file_path;
    SymbolId path_symbol_id = get_symbol_id_for_str_lit_token(path_tkn);
    const char* path = symbol_id_to_cstr(path_symbol_id);

    const std::filesystem::path parent = std::filesystem::path(path).parent_path();

    // DNE guard
    auto maybe_path = resolve_on_import_path(path, parent, this->args);
    if (!maybe_path.has_value()) {
        register_tokenwise_error(importer_id, path_tkn, ERR_IMPORTED_FILE_DOES_NOT_EXIST);
        ++this->fatal_error_count;
        return OptId<FileId>{};
    }
    return get_file(maybe_path.value());
}

} // namespace hir
