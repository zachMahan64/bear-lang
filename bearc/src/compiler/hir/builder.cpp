//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/builder.hpp"
#include "cli/args.h"
#include "compiler/ast/stmt.h"
#include "compiler/hir/file.hpp"
#include "compiler/hir/tables.hpp"
#include "utils/ansi_codes.h"
#include <cstddef>
#include <iostream>
#include <stdio.h>

extern "C" {

size_t hir_build(const bearc_args_t* args) {
    hir::Tables db = hir::builder::from_root_file(args->input_file_name, args);
    return db.error_count(); // todo return # errors
}

} // extern "C"
namespace hir {
namespace builder {

void try_print_info(const FileAst& file_ast, const bearc_args_t* args) {
    // --token-table
    if (args->flags[CLI_FLAG_TOKEN_TABLE]) {
        file_ast.print_token_table();
    }

    // --pretty-print
    if (args->flags[CLI_FLAG_PRETTY_PRINT]) {
        file_ast.pretty_print();
    }

    // display all comptime errors
    bool silent = args->flags[CLI_FLAG_SILENT];
    if (!silent) {
        file_ast.print_all_errors();
    }
}

// TODO parallelize and guard against infinite includes
void explore_imports(Tables& tables, FileId file_id) {
    const FileAst& root_ast = tables.file_asts.at(tables.files.at(file_id).ast_id);
    for (size_t i = 0; i < root_ast.root()->stmt.file.stmts.len; i++) {
        ast_stmt* curr = root_ast.root()->stmt.file.stmts.start[i];
        if (curr->type == AST_STMT_IMPORT) {
            FileId file = tables.emplace_file_from_path_tkn(curr->stmt.import.file_path);
            explore_imports(tables, file);
        }
    }
}

/// creates an HIR database from a file_name
Tables from_root_file(const char* root_file_path, const bearc_args_t* args) {
    Tables tables{};
    FileId root_id = tables.emplace_root_file(root_file_path);

    explore_imports(tables, root_id);

    for (const auto f : tables.files) {
        FileAst& ast = tables.file_asts.at(f.ast_id);
        tables.bump_parser_error_count(ast.error_count());
        try_print_info(ast, args);
    }
    // std::cout << tables.files.size() << '\n';
    if (tables.error_count() != 0) {
        if (!args->flags[CLI_FLAG_SILENT]) {
            printf("compilation terminated: %s'%s'\n%s", ansi_bold_white(), root_file_path,
                   ansi_reset());
        }
    }
    return tables;
}

} // namespace builder
} // namespace hir
