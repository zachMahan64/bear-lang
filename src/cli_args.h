#ifndef CLI_ARGS_H
#define CLI_ARGS_H

#include <stdbool.h>

// CLI DATA STRUCTS
#define CLI_ARGS_MAX_FILE_NAME_LENGTH 256
#define CLI_ARGS_MAX_FLAG_LENGTH 16
#define CLI_ARGS_NUM_VALID_LONG_FLAG_NAMES 4

typedef enum {
    NO_FLAG = '\0',
    ERROR = -1,
    BUILD = 'b',
    COMPILE = 'c',
    HELP = 'h',
    VERSION = 'v',
} cli_flag_e;

typedef struct {
    cli_flag_e flag;
    char file_name[CLI_ARGS_MAX_FILE_NAME_LENGTH];
} cli_args;

typedef struct {
    char name[CLI_ARGS_MAX_FLAG_LENGTH];
    cli_flag_e flag;
} cli_flag_long_mapping;

// CLI TABLES

extern cli_flag_long_mapping cli_flag_long_map[CLI_ARGS_NUM_VALID_LONG_FLAG_NAMES];

// CLI LOGIC
cli_args parse_cli_args(int argc, char** argv);
bool is_valid_cli_flag_short(char flag);
bool is_valid_cli_flag_long(const char* flag);
cli_flag_e search_cli_long_flags_for_valid_flag(const char* flag);
#endif // !CLI_ARGS_H
