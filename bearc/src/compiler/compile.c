//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/compile.h"
#include "cli/args.h"
#include "compiler/hir/builder.h"
#include <stddef.h>

int compile_file(const bearc_args_t* args) {
    int code = 0; // return error code if hit error
    code = (int)hir_build(args);
    return code;
}
