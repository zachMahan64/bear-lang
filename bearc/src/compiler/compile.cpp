//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/compile.h"
#include "cli/args.h"
#include "compiler/hir/context.hpp"
#include <stddef.h>

extern "C" {

int compile_file(const bearc_args_t* args) {
    hir::Context context{*args};
    context.try_print_info();
    return context.diagnostic_count();
}

} // extern "C"
