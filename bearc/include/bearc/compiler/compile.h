//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILE_H
#define COMPILE_H
#include "cli/args.h"

/**
 * compiles a source file given the file name
 *
 * returns the count of compiler errors
 */
int compile_file(const bearc_args_t* args);

#endif // !COMPILE_H
