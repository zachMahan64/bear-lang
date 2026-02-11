//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/builder.hpp"
#include "cli/args.h"
#include "compiler/hir/tables.hpp"
#include "utils/ansi_codes.h"
#include <stdio.h>

extern "C" {

size_t hir_build(const bearc_args_t* args) {
    hir::Tables db = hir::builder::from_file(args->input_file_name, args);
    return db.error_count(); // todo return # errors
}

} // extern "C"
namespace hir {
namespace builder {

/// creates an HIR database from a file_name
Tables from_file(const char* file_name, const bearc_args_t* args) {
    FileAst root_ast{file_name};
    // --token-table
    if (args->flags[CLI_FLAG_TOKEN_TABLE]) {
        root_ast.print_token_table();
    }

    // --pretty-print
    if (args->flags[CLI_FLAG_PRETTY_PRINT]) {
        root_ast.pretty_print();
    }

    // placeholder until we have a registry of multi-file compilation
    // display all comptime errors
    bool silent = args->flags[CLI_FLAG_SILENT];
    if (!silent) {
        root_ast.print_all_errors();
    }

    if (root_ast.error_count() != 0) {
        if (!args->flags[CLI_FLAG_SILENT]) {
            printf("compilation terminated: %s'%s'\n%s", ansi_bold_white(), file_name,
                   ansi_reset());
        }
    }
    Tables tables{};
    tables.bump_parser_error_count(root_ast.error_count());
    return tables; // TODO
}

} // namespace builder
} // namespace hir
