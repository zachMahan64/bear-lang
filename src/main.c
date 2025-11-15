// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "bearlang/bearlang.h"
#include "tests/test.h"
#include <stdio.h>

#define DEBUG

int main(int argc, char** argv) {
    test_run();
    return 0;
    int exit_code = br_interpreter_launch_cli(argc, argv);
    return exit_code;
}
