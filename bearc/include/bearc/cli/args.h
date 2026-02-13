//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef CLI_ARGS_H
#define CLI_ARGS_H

#include <stdbool.h>

#define CLI_ARGS_MAX_FILE_NAME_LENGTH 1024
#define CLI_ARGS_MAX_FLAG_LENGTH 16

#ifdef __cplusplus
extern "C" {
#endif

typedef enum cli_flag {
    CLI_FLAG_ERR_UNKNOWN,
    CLI_FLAG_ERR_DUPLICATE,
    CLI_FLAG_ERR_FILE_NAME_TOO_LONG,
    CLI_FLAG_HELP,
    CLI_FLAG_VERSION,
    CLI_FLAG_TOKEN_TABLE,
    CLI_FLAG_PRETTY_PRINT,
    CLI_FLAG_SILENT,
    CLI_FLAG_LIST_FILES,
    CLI_FLAG__NUM,
} cli_flag_e;

typedef struct bearc_args {
    bool flags[CLI_FLAG__NUM];
    char input_file_name[CLI_ARGS_MAX_FILE_NAME_LENGTH];
} bearc_args_t;

typedef struct {
    char name[CLI_ARGS_MAX_FLAG_LENGTH];
    cli_flag_e flag;
} cli_flag_long_mapping_t;

extern cli_flag_long_mapping_t cli_flag_long_map[];

bearc_args_t parse_cli_args(int argc, char** argv);

#ifdef __cplusplus
}
#endif

#endif // !CLI_ARGS_H
