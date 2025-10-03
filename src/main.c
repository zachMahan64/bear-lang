#include "bearlang.h"
#include "tests/test.h"
#include <stdio.h>

int main(int argc, char** argv) {
    test_run();
    int exit_code = br_interpreter_launch_cli(argc, argv);
    return exit_code;
}
