//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/tables.hpp"
#include "compiler/ast/stmt.h"
#include "compiler/hir/file.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/token.h"
#include "utils/log.hpp"
#include "llvm/ADT/SmallVector.h"
#include <atomic>
#include <cstring>
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
// TODO: make a non default-ctor that actually calculates estimate capacities necessary (find these
// number empirically then apply here)
Tables::Tables()
    : symbol_storage_arena{DEFAULT_SYMBOL_ARENA_CAP}, id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
      symbol_id_to_file_id_map{id_map_arena, DEFAULT_SYM_TO_FILE_ID_MAP_CAP},
      scopes{DEFAULT_SCOPE_VEC_CAP}, files{DEFAULT_FILE_VEC_CAP},
      file_asts{DEFAULT_FILE_AST_VEC_CAP}, scope_anons{DEFAULT_SCOPE_ANON_VEC_CAP},
      symbols{DEFAULT_SYMBOL_VEC_CAP}, execs{DEFAULT_EXEC_VEC_CAP}, defs{DEFAULT_DEF_VEC_CAP},
      file_ids{DEFAULT_FILE_ID_VEC_CAP}, importer_to_importees_vec{DEFAULT_FILE_VEC_CAP},
      importee_to_importers_vec{DEFAULT_FILE_VEC_CAP}, symbol_ids{DEFAULT_SYMBOL_VEC_CAP},
      exec_ids{DEFAULT_EXEC_VEC_CAP}, def_ids{DEFAULT_DEF_VEC_CAP},
      def_resolved{DEFAULT_DEF_VEC_CAP}, def_top_level_visited{DEFAULT_DEF_VEC_CAP},
      def_used{DEFAULT_DEF_VEC_CAP}, type_vec{DEFAULT_TYPE_VEC_CAP}, type_ids{DEFAULT_DEF_VEC_CAP},
      generic_param_ids{DEFAULT_GENERIC_PARAM_VEC_CAP},
      generic_params{DEFAULT_GENERIC_PARAM_VEC_CAP}, generic_arg_ids{DEFAULT_GENERIC_ARG_VEC_CAP},
      generic_args{DEFAULT_GENERIC_ARG_VEC_CAP}, symbol_map_arena{DEFAULT_SYMBOL_ARENA_CAP},
      str_to_symbol_id_map{symbol_map_arena} {}

// TODO obviously wrong temporary logic
Tables::Tables(Tables&& other) noexcept
    : symbol_storage_arena{DEFAULT_SYMBOL_ARENA_CAP}, id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
      symbol_id_to_file_id_map{id_map_arena, DEFAULT_SYM_TO_FILE_ID_MAP_CAP},
      scopes{DEFAULT_SCOPE_VEC_CAP}, files{DEFAULT_FILE_VEC_CAP},
      file_asts{DEFAULT_FILE_AST_VEC_CAP}, scope_anons{DEFAULT_SCOPE_ANON_VEC_CAP},
      symbols{DEFAULT_SYMBOL_VEC_CAP}, execs{DEFAULT_EXEC_VEC_CAP}, defs{DEFAULT_DEF_VEC_CAP},
      file_ids{DEFAULT_FILE_ID_VEC_CAP}, importer_to_importees_vec{DEFAULT_FILE_VEC_CAP},
      importee_to_importers_vec{DEFAULT_FILE_VEC_CAP}, symbol_ids{DEFAULT_SYMBOL_VEC_CAP},
      exec_ids{DEFAULT_EXEC_VEC_CAP}, def_ids{DEFAULT_DEF_VEC_CAP},
      def_resolved{DEFAULT_DEF_VEC_CAP}, def_top_level_visited{DEFAULT_DEF_VEC_CAP},
      def_used{DEFAULT_DEF_VEC_CAP}, type_vec{DEFAULT_TYPE_VEC_CAP}, type_ids{DEFAULT_DEF_VEC_CAP},
      generic_param_ids{DEFAULT_GENERIC_PARAM_VEC_CAP},
      generic_params{DEFAULT_GENERIC_PARAM_VEC_CAP}, generic_arg_ids{DEFAULT_GENERIC_ARG_VEC_CAP},
      generic_args{DEFAULT_GENERIC_ARG_VEC_CAP}, symbol_map_arena{DEFAULT_SYMBOL_ARENA_CAP},
      str_to_symbol_id_map{symbol_map_arena} {
    this->parse_error_count.store(other.parse_error_count.load(std::memory_order_relaxed));
    this->semantic_error_count.store(other.semantic_error_count.load(std::memory_order_relaxed));
}

void Tables::bump_parser_error_count(uint32_t cnt) noexcept {
    parse_error_count.fetch_add(cnt, std::memory_order_relaxed);
}

uint32_t Tables::error_count() const noexcept {
    return this->parse_error_count + this->semantic_error_count;
}

FileId Tables::get_file_from_path_tkn(token_t* tkn) {
    return get_file(get_symbol_id_for_str_lit_token(tkn));
}

SymbolId Tables::get_symbol_id(std::string_view str) {
    return get_symbol_id(str.data(), str.length());
}

SymbolId Tables::get_symbol_id(const char* start, size_t len) {
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
SymbolId Tables::get_symbol_id_for_tkn(token_t* tkn) {
    assert(tkn->type == TOK_IDENTIFIER);
    return get_symbol_id(tkn->start, tkn->len);
}
SymbolId Tables::get_symbol_id_for_str_lit_token(token_t* tkn) {
    assert(tkn->type == TOK_STR_LIT);
    return get_symbol_id(tkn->start + 1, tkn->len - 2); // trims outer quotes
}

FileId Tables::provide_root_file(const char* file_name) {
    SymbolId path_symbol = get_symbol_id(file_name, strlen(file_name));
    FileId file_id = get_file(path_symbol);
    files.at(file_id).load_state = file_import_state::done;
    return file_id;
}

FileId Tables::get_file(SymbolId path_symbol) {
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
    importee_to_importers_vec.bump();
    importer_to_importees_vec.bump();
    return file_id;
}

FileAstId Tables::emplace_ast(const char* file_name) {
    return this->file_asts.emplace_and_get_id(file_name);
}

const char* Tables::symbol_id_to_cstr(SymbolId id) { return this->symbols.cat(id).sv().data(); }

// TODO parallelize; track file dependencies (forward and reverse)
void Tables::explore_imports(FileId importer_file_id) {
    const FileAst& root_ast = this->file_asts.at(this->files.at(importer_file_id).ast_id);
    const ast_stmt* root = root_ast.root();
    if (!root) {
        return;
    }
    static constexpr HirSize EXPECTED_MAX_NUM_IMPORTS = 128;
    llvm::SmallVector<FileId, EXPECTED_MAX_NUM_IMPORTS> importees;

    for (size_t i = 0; i < root->stmt.file.stmts.len; i++) {
        ast_stmt* curr = root->stmt.file.stmts.start[i];
        if (curr->type == AST_STMT_IMPORT) {
            FileId importee_file_id = this->get_file_from_path_tkn(curr->stmt.import.file_path);
            // guard circularity
            if (files.at(importee_file_id).load_state != file_import_state::unvisited) {
                continue;
            }
            // add importee to vec
            importees.emplace_back(importee_file_id);
            files.at(importee_file_id).load_state = file_import_state::in_progress;
            this->explore_imports(importee_file_id);
            files.at(importee_file_id).load_state = file_import_state::done;
        }
    }
    importer_to_importees_vec.at(importer_file_id) = file_ids.freeze_small_vec(importees);
}

} // namespace hir
