//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "tests/test.h"
#include "compiler/compile.h"
#include "utils/string.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {
    br_test_result_t result = test_parser();
    printf("tests passed: %zu/%zu\n", result.cnt_success, result.cnt_total);
    return 0;
}

br_test_result_t test_parser(void) {
    br_test_result_t result = {.cnt_success = 0, .cnt_total = 0};
    for (char i = '0'; i <= '3'; i++) {
        for (char k = '0'; k <= '9'; k++) {
            string_t str = string_create_and_reserve(5);
            string_push_cstr(&str, "tests/");
            string_push_char(&str, i);
            string_push_char(&str, k);
            string_push_cstr(&str, ".br");
            if (compile_file(string_data(&str)) == 0) {
                ++result.cnt_success;
            }
            ++result.cnt_total;
            string_destroy(&str);
        }
    }
    return result;
}
