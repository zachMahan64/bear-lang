//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/compile.h"
#include "cli/args.h"
#include "compiler/ast/ast.h"
#include "compiler/ast/printer.h"
#include "compiler/debug.h"
#include "compiler/diagnostics/error_list.h"
#include "utils/ansi_codes.h"
#include <stddef.h>
#include <stdio.h>

int compile_file(const bearc_args_t* args) {
    int code = 0; // return error code if hit error

    const char* file_name = args->input_file_name;
    br_ast_t ast = ast_create_from_file(file_name);

    // --token-table
    if (args->flags[CLI_FLAG_TOKEN_TABLE]) {
        print_out_src_buffer(&ast.src_buffer);
        print_out_tkn_table(&ast.tokens);
    }

    // --pretty-print
    if (args->flags[CLI_FLAG_PRETTY_PRINT]) {
        pretty_print_stmt(ast.file_stmt_root_node);
    }
    // ----------------------------------------------------

    // placeholder until we have a registry of multi-file compilation
    // display all comptime errors
    bool silent = args->flags[CLI_FLAG_SILENT];
    compiler_error_list_t* error_list = &ast.error_list;
    if (!silent) {
        compiler_error_list_print_all(error_list);
    }
    if (!compiler_error_list_empty(error_list)) {
        if (!silent) {
            printf("compilation terminated: %s'%s'\n%s", ansi_bold_white(), file_name,
                   ansi_reset());
        }
        code = (int)compiler_error_list_count(error_list);
    }

    // clean up resources
    ast_destroy(&ast);
    return code;
}
