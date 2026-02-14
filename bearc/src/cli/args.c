//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "cli/args.h"
#include "stddef.h"
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

bool is_valid_cli_flag_long(const char* flag);
cli_flag_e search_cli_long_flags_for_valid_flag(const char* flag);

const cli_flag_e short_flag_map[256] = {['h'] = CLI_FLAG_HELP, ['v'] = CLI_FLAG_VERSION};

// LONG FLAG NAME MAP
cli_flag_long_mapping_t cli_flag_long_map[] = {{"help", CLI_FLAG_HELP},
                                               {"version", CLI_FLAG_VERSION},
                                               {"pretty-print", CLI_FLAG_PRETTY_PRINT},
                                               {"token-table", CLI_FLAG_TOKEN_TABLE},
                                               {"silent", CLI_FLAG_SILENT},
                                               {"list-files", CLI_FLAG_LIST_FILES},
                                               {"import-path", CLI_FLAG_IMPORT_PATH}};
static bool is_valid_cli_flag_short(const char* arg) {
    return strlen(arg) == 2 && arg[0] == '-' && short_flag_map[(unsigned char)arg[1]];
}
static bool check_duplicate(bearc_args_t* args, cli_flag_e flag) {
    if (args->flags[flag]) {
        args->flags[CLI_FLAG_ERR_DUPLICATE] = true;
        return true;
    }
    return false;
}

static bool is_flag(const char* arg) {
    return is_valid_cli_flag_long(arg) || is_valid_cli_flag_short(arg);
}

bearc_args_t parse_cli_args(int argc, char** argv) {
    bearc_args_t args
        = {.flags = {0}, .input_file_name = NULL, .import_paths = {0}, .import_path_cnt = 0};
    int count = 1;
    while (count < argc) {
        // attempt to extract the char from a -<something> flag
        if (is_valid_cli_flag_short(argv[count])) {
            cli_flag_e flag = short_flag_map[(unsigned char)argv[count][1]];
            if (check_duplicate(&args, flag)) {
                return args;
            }
            args.flags[flag] = true;
        }
        // extract from a --<something> flag
        else if (is_valid_cli_flag_long(argv[count])) {
            cli_flag_e flag = search_cli_long_flags_for_valid_flag(argv[count]);
            if (check_duplicate(&args, flag)) {
                return args;
            }
            if (flag == CLI_FLAG_IMPORT_PATH) {
                int imp_cnt = 0;
                count++;
                while (imp_cnt < CLI_ARGS_MAX_IMPORT_PATH_COUNT && count < argc
                       && !is_flag(argv[count])) {
                    args.import_paths[imp_cnt] = argv[count];
                    imp_cnt++;
                    count++;
                }
                args.import_path_cnt = imp_cnt;
                // backtrack overshoot (since count will be incremented again at the end of the
                // loop)
                count--;
            }
            args.flags[flag] = true;
        } else {
            if (strlen(argv[count]) >= 2 && argv[count][0] == '-') {
                args.flags[CLI_FLAG_ERR_UNKNOWN] = true; // invalid flag
            } else if (!args.input_file_name) {
                // try to interpret as filename, deal with faulty filenames later
                if (strlen(argv[count]) < CLI_ARGS_MAX_FILE_NAME_LENGTH - 1) {
                    args.input_file_name = argv[count];
                } else {
                    args.flags[CLI_FLAG_ERR_FILE_NAME_TOO_LONG] = true;
                }
            } else {
                args.flags[CLI_FLAG_ERR_TOO_MANY_INPUT_FILES] = true;
            }
        }
        ++count;
    }
    return args;
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

    for (size_t i = 0; i < (sizeof cli_flag_long_map / sizeof cli_flag_long_map[0]); i++) {
        if (strcmp(cli_flag_long_map[i].name, flag_long_name) == 0) {
            return true;
        }
    }
    return false;
}

// linear lookup, which is fast until that option list gets real long
cli_flag_e search_cli_long_flags_for_valid_flag(const char* flag) {
    size_t FLAG_PREFIX_LENGTH = 2;
    for (size_t i = 0; i < (sizeof cli_flag_long_map / sizeof cli_flag_long_map[0]); i++) {
        if (strcmp(cli_flag_long_map[i].name, (flag + FLAG_PREFIX_LENGTH)) == 0) {
            return cli_flag_long_map[i].flag;
        }
    }
    return CLI_FLAG_ERR_UNKNOWN;
}
