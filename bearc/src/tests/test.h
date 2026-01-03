//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef TEST_H
#define TEST_H

#include <stddef.h>

typedef struct {
    /// subtest total count
    size_t cnt_total;
    /// subtest success count
    size_t cnt_success;
} br_test_result_t;

br_test_result_t test_parser(void);

#endif // !TEST_H
