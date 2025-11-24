// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "bearc/bearc.h"
#include <stdio.h>

int main(int argc, char** argv) {
    int exit_code = br_launch_cli(argc, argv);
    return exit_code;
}
