//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/file.hpp"
#include "compiler/ast/ast.h"
#include "compiler/ast/printer.h"
#include "compiler/debug.h"
namespace hir {
FileAst::FileAst(const char* file_name) : ast(ast_create_from_file(file_name)) {}
FileAst::~FileAst() { ast_destroy(&this->ast); }
const src_buffer& FileAst::buffer() const noexcept { return this->ast.src_buffer; }
const compiler_error_list_t& FileAst::error_list() const noexcept { return this->ast.error_list; }
const ast_stmt_t* FileAst::root() const noexcept { return this->ast.file_stmt_root_node; }
void FileAst::pretty_print() const { pretty_print_stmt(this->root()); }
void FileAst::print_all_errors() const { compiler_error_list_print_all(&this->ast.error_list); }
size_t FileAst::error_count() const { return compiler_error_list_count(&this->ast.error_list); }
void FileAst::print_token_table() const {
    print_out_src_buffer(&ast.src_buffer);
    print_out_tkn_table(&ast.tokens);
}
} // namespace hir
