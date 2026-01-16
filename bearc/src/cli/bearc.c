//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "bearc.h"
#include "cli/args.h"
#include "compiler/compile.h"
#include "compiler/token.h"
#include "utils/ansi_codes.h"
#include "utils/file_io.h"
#include "utils/log.h"
#include <stdio.h>
#include <string.h>

// private cli error struct
#define BR_CLI_DO_FN_ERROR_MSG_LEN 64
typedef struct {
    int error_code;
    char error_message[BR_CLI_DO_FN_ERROR_MSG_LEN];
} cli_error_status;

// private cli functions
void do_cli_help(void);
void do_cli_version(void);
cli_error_status do_cli_compile(const cli_args_t* args);
cli_error_status do_cli_build(const cli_args_t* args);
cli_error_status do_cli_no_args(void);
void do_cli_announce_error(void);

int br_launch_cli(int argc, char** argv) {
    // parse and do preliminary validatation on args, convert to max 1 flag, 1 filename
    cli_args_t args = parse_cli_args(argc, argv);

    // error that dispatched cli functions can return, default to inoffensive values
    cli_error_status error_status = {0, ""};

    LOG_STR_NNL("File name: ");
    LOG_STR(args.file_name);
    LOG_STR_NNL("Flag: ");
    LOG_CHAR(args.flag);
    if (args.flag == ERROR) {
        do_cli_announce_error();
        return -1;
    }
    if (strlen(args.file_name) != 0 && !file_exists(args.file_name)) {
        printf("%serror:%s file does not exist: %s'%s'\n%s", ansi_bold_red(), ansi_reset(),
               ansi_bold(), args.file_name, ansi_reset());
        return -1;
    }
    if (args.flag == HELP) {
        do_cli_help();
    } else if (args.flag == VERSION) {
        do_cli_version();
    } else if (args.flag == NO_FLAG && strlen(args.file_name)) {
        error_status = do_cli_compile(&args);
    } else if (args.flag == NO_FLAG && !strlen(args.file_name)) {
        error_status = do_cli_no_args();
    }
    if (error_status.error_code < 0) {
        // printf("error: %s", error_status.error_message);
        return error_status.error_code;
    }
    return 0;
}

void do_cli_help(void) {
    const char* help_message = "usage:\n"
                               "        bearc <file_name> <flag> \n"
                               "flags:\n"
                               "        [--version | -v]\n"
                               "        [--help | -h]\n";

    printf("%s", help_message);
}

void do_cli_version(void) { puts("bearc v0.0.1"); }

cli_error_status do_cli_compile(const cli_args_t* args) {
    printf("%s(bearc) this feature is a work-in-progress\n%s", ansi_bold_white(), ansi_reset());
    cli_error_status error_status = {0, ""};
    error_status.error_code = compile_file(args->file_name);
    token_maps_free(); // after all operations involving token lookups are done
    return error_status;
}
cli_error_status do_cli_no_args(void) {
    puts("(bearc) no arguments specified.");
    cli_error_status error_status = {0, ""};
    return error_status;
}
void do_cli_announce_error(void) { puts("(bearc) invalid flag"); }
