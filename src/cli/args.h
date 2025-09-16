#ifndef CLI_ARGS_H
#define CLI_ARGS_H

#define MAX_FILE_NAME_LENGTH 256

typedef enum {
    BUILD = 'b',
    COMPILE = 'c',
    HELP = 'h',
    VERSION = 'v',
} cli_flag;

typedef struct {
    cli_flag flag;
    char file_name[MAX_FILE_NAME_LENGTH];
} cli_args;

cli_args parse_cli_args(int argc, char** argv);

#endif // !CLI_ARGS_H
