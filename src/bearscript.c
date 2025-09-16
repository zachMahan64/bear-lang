#include "bearscript.h"
#include "cli/args.h"
int bs_interpreter_launch_cli(int argc, char** argv) {
    cli_args args = parse_cli_args(argc, argv);
    return 0;
}
