#include "bearscript.h"
#include "cli/args.h"
#include "log.h"
#include <stdio.h>

// private cli functions
void do_cli_help(void);
void do_cli_version(void);
// TODO add other cli functions

int bs_interpreter_launch_cli(int argc, char** argv) {
    cli_args args = parse_cli_args(argc, argv);
    LOG_STR_NNL("[DEBUG] File name: ");
    LOG_STR(args.file_name);
    LOG_STR_NNL("[DEBUG] Flag: ");
    LOG_CHAR(args.flag);
    if (args.flag == ERROR) {
        return -1;
    }
    switch (args.flag) {
    case (HELP): {
        do_cli_help();
        break;
    }
    case (VERSION): {
        do_cli_version();
        break;
    }
    default: {
        break;
    }
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
