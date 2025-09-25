#include "cli_args.h"
#include <stdbool.h>
#include <stdio.h>
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
        else if (is_valid_cli_flag_long(argv[i])) {
            args.flag = search_cli_long_flags_for_valid_flag(argv[i]);
        } else {
            if (strlen(argv[i]) >= 2 && argv[i][0] == '-') {
                args.flag = ERROR; // invalid flag
            } else {
                // try to interpret as filename, deal with faulty filenames later
                strcpy(args.file_name, argv[i]);
            }
        }
    }
    return args;
}

bool is_valid_cli_flag_short(const char flag) {
    return (flag == 'b' || flag == 'c' || flag == 'h' || flag == 'v');
}

bool is_valid_cli_flag_long(const char* flag) {
    size_t FLAG_PREFIX_LENGTH = 2;

    if (strlen(flag) < FLAG_PREFIX_LENGTH + 1) {
        return false; // too short to be valid
    }
    if (flag[0] != '-' || flag[1] != '-') {
        return false; // must start with --
    }

    const char* flag_long_name = flag + FLAG_PREFIX_LENGTH;

    if (strlen(flag_long_name) >= CLI_ARGS_MAX_FLAG_LENGTH - 1) {
        return false; // would overflow buffer if copied
    }

    for (int i = 0; i < CLI_ARGS_NUM_VALID_LONG_FLAG_NAMES; i++) {
        if (strcmp(cli_flag_long_map[i].name, flag_long_name) == 0) {
            return true;
        }
    }
    return false;
}

cli_flag_e search_cli_long_flags_for_valid_flag(const char* flag) {
    size_t FLAG_PREFIX_LENGTH = 2;
    for (int i = 0; i < CLI_ARGS_NUM_VALID_LONG_FLAG_NAMES; i++) {
        if (strcmp(cli_flag_long_map[i].name, (flag + FLAG_PREFIX_LENGTH)) == 0) {
            return cli_flag_long_map[i].flag;
        }
    }
    return ERROR;
}
