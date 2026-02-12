//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/tables.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/token.h"
#include <atomic>
#include <cstring>
#include <stddef.h>
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
    : symbol_arena{DEFAULT_SYMBOL_ARENA_CAP}, id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
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
      generic_args{DEFAULT_GENERIC_ARG_VEC_CAP} {}

// TODO obviously wrong temporary logic
Tables::Tables(Tables&& other) noexcept
    : symbol_arena{DEFAULT_SYMBOL_ARENA_CAP}, id_map_arena{DEFAULT_ID_MAP_ARENA_CAP},
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
      generic_args{DEFAULT_GENERIC_ARG_VEC_CAP} {
    this->parse_error_count.store(other.parse_error_count.load(std::memory_order_relaxed));
    this->semantic_error_count.store(other.semantic_error_count.load(std::memory_order_relaxed));
}

void Tables::bump_parser_error_count(uint32_t cnt) noexcept {
    parse_error_count.fetch_add(cnt, std::memory_order_relaxed);
}

uint32_t Tables::error_count() const noexcept {
    return this->parse_error_count + this->semantic_error_count;
}

FileId Tables::emplace_file_from_path_tkn(token_t* tkn) {
    return emplace_file(emplace_str_literal_symbol(tkn));
}

SymbolId Tables::emplace_symbol(const char* start, size_t len) {
    /// this->str_to_symbol_id_map ...; TODO ADD TO A HASHMAP ONCE IMPLEMENTED, THIS WILL ALSO GUARD
    /// AGAINST DUPLICATES
    char* sym_data = this->symbol_arena.alloc_as<char*>(len + 1); // +1 for null-term
    memcpy(sym_data, start, len);
    sym_data[len] = '\0'; // null-term
    return this->symbols.emplace_and_get_id(std::string_view{sym_data, len});
}
SymbolId Tables::emplace_symbol_from_token(token_t* tkn) {
    assert(tkn->type == TOK_IDENTIFIER);
    return emplace_symbol(tkn->start, tkn->len);
}
SymbolId Tables::emplace_str_literal_symbol(token_t* tkn) {
    assert(tkn->type == TOK_STR_LIT);
    return emplace_symbol(tkn->start + 1, tkn->len - 2); // trims outer quotes
}

FileId Tables::emplace_root_file(const char* file_name) {
    SymbolId path_symbol = emplace_symbol(file_name, strlen(file_name));
    FileAstId ast_id = this->file_asts.emplace_and_get_id(file_name);
    return this->files.emplace_and_get_id(path_symbol, ast_id);
}

FileId Tables::emplace_file(SymbolId path_symbol) {
    FileAstId ast_id = this->file_asts.emplace_and_get_id(symbol_id_to_raw_str(path_symbol));
    return this->files.emplace_and_get_id(path_symbol, ast_id);
}

FileAstId Tables::emplace_ast(const char* file_name) {
    return this->file_asts.emplace_and_get_id(file_name);
}

const char* Tables::symbol_id_to_raw_str(SymbolId id) { return this->symbols.cat(id).sv().data(); }

} // namespace hir
// namespace hir
