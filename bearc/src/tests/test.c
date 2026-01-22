//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "tests/test.h"
#include "cli/args.h"
#include "compiler/compile.h"
#include "compiler/token.h"
#include "string.h"
#include "utils/ansi_codes.h"
#include "utils/vector.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

static bearc_args_t args;

int main(int argc, char** argv) {

    args = parse_cli_args(argc, argv);

    br_test_result_t total = test_total_init();
    vector_t results = vector_create_and_reserve(sizeof(br_test_result_t), 8);
    *((br_test_result_t*)vector_emplace_back(&results)) = test_parser();
    printf("%s -----------------------------%s\n", ansi_bold_white(), ansi_reset());
    printf(" |%s        test results       %s|\n", ansi_bold_blue(), ansi_bold_white());
    printf("%s -----------------------------%s\n", ansi_bold_white(), ansi_reset());
    for (size_t i = 0; i < results.size; i++) {
        br_test_result_t* res = (br_test_result_t*)vector_at(&results, i);
        print_result(res);
        test_tally(&total, res);
    }
    printf("%s -----------------------------%s\n", ansi_bold_white(), ansi_reset());
    print_result(&total);
    vector_destroy(&results);
    token_maps_free(); // after all operations involving token lookups are done
    return !(total.cnt_total == total.cnt_success);
}
/*
 * macros to easily test tests in the form tests/00.br
 */

#define TEST_INIT(_name)                                                                           \
    br_test_result_t br_test_result = {.cnt_success = 0, .cnt_total = 0, .name = (_name)};         \
    int true_cnt = 0;
#define ASSERT_EQ_ERR(file_name, err_cnt)                                                          \
    strcpy(args.input_file_name, "tests/" file_name ".br");                                        \
    true_cnt = compile_file(&args);                                                                \
    if (true_cnt == (err_cnt)) {                                                                   \
        br_test_result.cnt_success++;                                                              \
    } else {                                                                                       \
        printf("\n%sTEST FAILED %s('"                                                              \
               "tests/" file_name ".br"                                                            \
               "'): expected %d errors, got %d %s\n\n",                                            \
               ansi_bold_red(), ansi_bold_white(), err_cnt, true_cnt, ansi_reset());               \
    }                                                                                              \
    br_test_result.cnt_total++;
#define ASSERT_GTE_ERR(file_name, err_cnt)                                                         \
    if (compile_file({"tests/" file_name ".br", {0}) >= (err_cnt)) {                                     \
        br_test_result.cnt_success++;                                                              \
    };                                                                                             \
    br_test_result.cnt_total++;
#define ASSERT_ERR(file_name)                                                                      \
    if (compile_file("tests/" file_name ".br") > 0) {                                              \
        br_test_result.cnt_success++;                                                              \
    };                                                                                             \
    br_test_result.cnt_total++;
#define TEST_RESULT br_test_result

br_test_result_t test_parser(void) {
    TEST_INIT("parser");
    ASSERT_EQ_ERR("00", 6);
    ASSERT_EQ_ERR("01", 0);
    ASSERT_EQ_ERR("02", 5);
    ASSERT_EQ_ERR("03", 0);
    ASSERT_EQ_ERR("04", 8);
    ASSERT_EQ_ERR("05", 0);
    ASSERT_EQ_ERR("06", 14);
    ASSERT_EQ_ERR("07", 10);
    ASSERT_EQ_ERR("08", 0);
    ASSERT_EQ_ERR("09", 13);
    ASSERT_EQ_ERR("10", 1);
    ASSERT_EQ_ERR("11", 0);
    ASSERT_EQ_ERR("12", 0);
    ASSERT_EQ_ERR("13", 1);
    ASSERT_EQ_ERR("14", 0);
    ASSERT_EQ_ERR("15", 0);
    ASSERT_EQ_ERR("16", 0);
    ASSERT_EQ_ERR("17", 0);
    ASSERT_EQ_ERR("18", 6);
    ASSERT_EQ_ERR("19", 3);
    ASSERT_EQ_ERR("20", 0);
    ASSERT_EQ_ERR("21", 5);
    ASSERT_EQ_ERR("22", 1);
    ASSERT_EQ_ERR("23", 1);
    ASSERT_EQ_ERR("24", 1);
    ASSERT_EQ_ERR("25", 2);
    ASSERT_EQ_ERR("26", 1);
    ASSERT_EQ_ERR("27", 5);
    ASSERT_EQ_ERR("28", 0);
    ASSERT_EQ_ERR("29", 0);
    ASSERT_EQ_ERR("30", 0);
    ASSERT_EQ_ERR("31", 0);
    ASSERT_EQ_ERR("32", 0);
    ASSERT_EQ_ERR("33", 0);
    ASSERT_EQ_ERR("34", 1);
    ASSERT_EQ_ERR("35", 0);
    ASSERT_EQ_ERR("36", 0);
    ASSERT_EQ_ERR("37", 0);
    ASSERT_EQ_ERR("38", 0);
    ASSERT_EQ_ERR("39", 0);
    ASSERT_EQ_ERR("40", 3);
    ASSERT_EQ_ERR("41", 0);
    ASSERT_EQ_ERR("42", 1);
    ASSERT_EQ_ERR("43", 0);
    ASSERT_EQ_ERR("44", 0);

    return TEST_RESULT;
}

br_test_result_t test_total_init(void) {
    br_test_result_t res = {.cnt_success = 0, .cnt_total = 0, .name = "total"};
    return res;
}

void test_tally(br_test_result_t* total, br_test_result_t* new_test) {
    total->cnt_total += new_test->cnt_total;
    total->cnt_success += new_test->cnt_success;
}

void print_result(br_test_result_t* res) {
    size_t succ = res->cnt_success;
    size_t tot = res->cnt_total;
    const char* color = ansi_bold_white();
    if (succ == tot) {
        color = ansi_bold_green();
    } else if (tot != 0 && ((double)succ / (double)tot >= .9)) {
        color = ansi_bold_yellow();
    } else {
        color = ansi_bold_red();
    }
    printf("  %s%-18s   %s%zu/%zu%s\n", ansi_bold(), res->name, color, succ, tot, ansi_reset());
}
