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
#include <stdio.h>
#include <string.h>

// private cli error struct
#define BR_CLI_DO_FN_ERROR_MSG_LEN 64
typedef struct {
    int error_code;
    char error_message[BR_CLI_DO_FN_ERROR_MSG_LEN];
} cli_error_status;

// private cli functions
void cli_help(void);
void cli_version(void);
cli_error_status cli_compile(const bearc_args_t* args);
cli_error_status cli_build(const bearc_args_t* args);
void cli_no_args(void);
void cli_announce_unknown_flag(void);
void warn_duplicate_flag(void);
// checks if the args are otherwise empty besides the specified flag
bool cli_args_otherwise_empty(bearc_args_t* args, cli_flag_e flag);

void do_cli_announce_incompatible_flags(void);

int br_launch_cli(int argc, char** argv) {
    ansi_init();
    // parse and do preliminary validatation on args, convert to max 1 flag, 1 filename
    bearc_args_t args = parse_cli_args(argc, argv);

    // error that dispatched cli functions can return, default to inoffensive values
    cli_error_status error_status = {0, ""};
    if (args.flags[CLI_FLAG_ERR_UNKNOWN]) {
        cli_announce_unknown_flag();
        return -1;
    }
    if (args.flags[CLI_FLAG_ERR_DUPLICATE]) {
        warn_duplicate_flag();
        return -1;
    }
    if (strlen(args.input_file_name) != 0 && !file_exists(args.input_file_name)) {
        printf("%serror:%s file does not exist: %s'%s'\n%s", ansi_bold_red(), ansi_reset(),
               ansi_bold(), args.input_file_name, ansi_reset());
        return -1;
    }

    // no compilation options
    if (args.flags[CLI_FLAG_HELP]) {
        if (!cli_args_otherwise_empty(&args, CLI_FLAG_HELP)) {
            do_cli_announce_incompatible_flags();
            return -1;
        }
        cli_help();
    }
    if (args.flags[CLI_FLAG_VERSION]) {
        if (!cli_args_otherwise_empty(&args, CLI_FLAG_VERSION)) {
            do_cli_announce_incompatible_flags();
            return -1;
        }
        cli_version();
    }

    if (args.flags[CLI_FLAG_ERR_FILE_NAME_TOO_LONG]) {

        printf("%s(bearc)%s file name exessively long\n", ansi_bold(), ansi_reset());
    }

    // compilation
    if (strlen(args.input_file_name)) {
        error_status = cli_compile(&args);
    } else if (!strlen(args.input_file_name)
               && cli_args_otherwise_empty(&args, CLI_FLAG_ERR_UNKNOWN)) {
        cli_no_args();
    }
    if (error_status.error_code < 0) {
        // printf("error: %s", error_status.error_message);
        return error_status.error_code;
    }
    return 0;
}

void cli_help(void) {
    const char* help_message = "usage:\n"
                               "        bearc <file_name> <flags> \n"
                               "flags:\n"
                               "        [--version | -v]\n"
                               "        [--help | -h]\n";

    printf("%s", help_message);
}

void cli_version(void) { puts("bearc v0.0.1"); }

cli_error_status cli_compile(const bearc_args_t* args) {
    cli_error_status error_status = {0, ""};
    error_status.error_code = compile_file(args);
    token_maps_free(); // after all operations involving token lookups are done
    return error_status;
}

void cli_no_args(void) {
    cli_version();
    printf("run '%sbearc --help%s' to see available operations.\n", ansi_bold(), ansi_reset());
}
void cli_announce_unknown_flag(void) {
    printf("%s(bearc)%s unknown flag(s)\n", ansi_bold(), ansi_reset());
}

void warn_duplicate_flag(void) {
    printf("%s(bearc)%s duplicate flag(s)\n", ansi_bold(), ansi_reset());
}

void do_cli_announce_incompatible_flags(void) {
    printf("%s(bearc)%s incompatible flags\n", ansi_bold(), ansi_reset());
}

bool cli_args_otherwise_empty(bearc_args_t* args, cli_flag_e flag) {
    if (strlen(args->input_file_name) != 0) {
        return false;
    }
    for (size_t i = 0; i < CLI_FLAG__NUM; i++) {
        if (args->flags[i] == true && i != flag) {
            return false;
        }
    }
    return true;
}
