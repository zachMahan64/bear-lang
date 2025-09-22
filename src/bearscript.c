#include "bearscript.h"
#include "cli/args.h"
#include "log.h"
#include <stdio.h>
#include <string.h>

// private cli error struct
#define BS_CLI_DO_FN_ERROR_MSG_LEN 64
typedef struct {
    int error_code;
    char error_message[BS_CLI_DO_FN_ERROR_MSG_LEN];
} cli_error_status;

// private cli functions
void do_cli_help(void);
void do_cli_version(void);
cli_error_status do_cli_compile(const cli_args* args);
cli_error_status do_cli_build(const cli_args* args);
cli_error_status do_cli_interpret_file(void);
cli_error_status do_cli_interpret_live(void);
void do_cli_announce_error(cli_args* args);

int bs_interpreter_launch_cli(int argc, char** argv) {
    cli_args args = parse_cli_args(argc, argv);

    cli_error_status error_status = {
        0, ""}; // error that dispatched cli functions can return, default to inoffensive values

    LOG_STR_NNL("[DEBUG] File name: ");
    LOG_STR(args.file_name);
    LOG_STR_NNL("[DEBUG] Flag: ");
    LOG_CHAR(args.flag);
    if (args.flag == ERROR) {
        do_cli_announce_error(&args);
        return -1;
    }
    if (args.flag == HELP) {
        do_cli_help();
    } else if (args.flag == VERSION) {
        do_cli_version();
    } else if (args.flag == COMPILE) {
        error_status = do_cli_compile(&args);
    } else if (args.flag == BUILD) {
        error_status = do_cli_build(&args);
    } else if (args.flag == NO_FLAG && !strlen(args.file_name)) {
        error_status = do_cli_interpret_live();
    } else if (args.flag == NO_FLAG && strlen(args.file_name)) {
        error_status = do_cli_interpret_file();
    }
    if (error_status.error_code < 0) {
        printf("[ERROR] %s", error_status.error_message);
        return error_status.error_code;
    }
    return 0;
}

void do_cli_help(void) {
    const char* help_message = "usage:\n"
                               "        bs <file_name> <flag> \n"
                               "flags:\n"
                               "        [--compile | -c]\n"
                               "        [--version | -v]\n"
                               "        [--build | -b]\n"
                               "        [--help | -h]\n"
                               "        [<none>] -> live interpret\n";

    printf("%s", help_message);
}

void do_cli_version(void) { puts("BearScript v0.0.1"); }

cli_error_status do_cli_compile(const cli_args* args) {
    // TODO
    puts("(compile) This feature is WIP.");
    cli_error_status error_status = {0, ""};
    return error_status;
}
cli_error_status do_cli_build(const cli_args* args) {
    // TODO
    puts("(build) This feature is WIP.");
    cli_error_status error_status = {0, ""};
    return error_status;
}
cli_error_status do_cli_interpret_file(void) {
    // TODO
    puts("(interpret file) This feature is WIP.");
    cli_error_status error_status = {0, ""};
    return error_status;
}
cli_error_status do_cli_interpret_live(void) {
    // TODO
    puts("(live interpret) This feature is WIP.");
    cli_error_status error_status = {0, ""};
    return error_status;
}
void do_cli_announce_error(cli_args* args) { puts("[ERROR] Invalid flag"); }
