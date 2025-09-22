#include "bearscript.h"
#include <stdio.h>

int main(int argc, char** argv) {
    printf("[INFO] Launched BearScript CLI\n");
    int exit_code = bs_interpreter_launch_cli(argc, argv);
    return exit_code;
}
