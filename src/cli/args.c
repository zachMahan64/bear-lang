#include "cli/args.h"
#include <string.h>

// LONG FLAG NAME MAP

cli_flag_long_mapping cli_flag_long_map[CLI_ARGS_NUM_VALID_LONG_FLAG_NAMES] = {
    {"build", BUILD}, {"compile", COMPILE}, {"help", HELP}, {"version", VERSION}};

cli_args parse_cli_args(int argc, char** argv) {
    cli_args args = {NO_FLAG, ""}; // WIP
    if (argc > 3) {
        args.flag = ERROR;
        return args;
    }
    for (int i = 1; i < argc; ++i) {
        // attempt to extract the char from a -<something> flag
        if (strlen(argv[i]) == 2 && argv[i][0] == '-' && is_valid_cli_flag_short(argv[i][1])) {
            args.flag = (cli_flag_e)argv[i][1];
        }
        // extract from a --<something> flag
        // TODO, need to write the is_valid_cli_flag_long function
    }
    return args;
}

bool is_valid_cli_flag_short(char flag) {
    return (flag == 'b' || flag == 'c' || flag == 'h' || flag == 'v');
}
