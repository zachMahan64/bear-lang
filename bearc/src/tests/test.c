//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "tests/test.h"
#include "cli/args.h"
#include "compiler/token.h"
#include "string.h"
#include "utils/ansi_codes.h"
#include "utils/vector.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bearc_args_t args;

int main(int argc, char** argv) {

    ansi_init();

    args = parse_cli_args(argc, argv);

    br_test_result_t total = test_total_init();
    vector_t results = vector_create_and_reserve(sizeof(br_test_result_t), 8);

    // add all tests here ----------
    *((br_test_result_t*)vector_emplace_back(&results)) = test_parser();
    *((br_test_result_t*)vector_emplace_back(&results)) = test_hir();
    *((br_test_result_t*)vector_emplace_back(&results)) = test_context_db();
    // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

    printf("%s -----------------------------%s\n", ansi_bold_reset(), ansi_reset());
    printf(" |%s        test results       %s|\n", ansi_bold_blue(), ansi_bold_reset());
    printf("%s -----------------------------%s\n", ansi_bold_reset(), ansi_reset());
    for (size_t i = 0; i < results.size; i++) {
        br_test_result_t* res = (br_test_result_t*)vector_at(&results, i);
        print_result(res);
        test_tally(&total, res);
    }
    printf("%s -----------------------------%s\n", ansi_bold_reset(), ansi_reset());
    print_result(&total);
    vector_destroy(&results);
    token_maps_free(); // after all operations involving token lookups are done
    return (int)(total.cnt_total - total.cnt_success);
}
/*
 * macros to easily test tests in the form tests/00.br
 */

br_test_result_t test_parser(void) {
    TEST_INIT("parser");
    char* parser_args[]
        = {"bearc",
           "--parse-only",
           /*"--pretty-print"*/}; // pretty-print to catch unexpectly null fields and other
                                  // malformation-related errors that could cause segfaults/crashes
                                  // this makes tests sustainally slower, so only enable when
                                  // parsing updates are made to verify correctness
    TEST_SET_ARGS(parser_args);
    ASSERT_EQ_ERR("parser/00", 3);
    ASSERT_EQ_ERR("parser/01", 0);
    ASSERT_EQ_ERR("parser/02", 2);
    ASSERT_EQ_ERR("parser/03", 0);
    ASSERT_EQ_ERR("parser/04", 4);
    ASSERT_EQ_ERR("parser/05", 0);
    ASSERT_EQ_ERR("parser/06", 10);
    ASSERT_EQ_ERR("parser/07", 6);
    ASSERT_EQ_ERR("parser/08", 0);
    ASSERT_EQ_ERR("parser/09", 7);
    ASSERT_EQ_ERR("parser/10", 3);
    ASSERT_EQ_ERR("parser/11", 0);
    ASSERT_EQ_ERR("parser/12", 0);
    ASSERT_EQ_ERR("parser/13", 1);
    ASSERT_EQ_ERR("parser/14", 0);
    ASSERT_EQ_ERR("parser/15", 0);
    ASSERT_EQ_ERR("parser/16", 0);
    ASSERT_EQ_ERR("parser/17", 0);
    ASSERT_EQ_ERR("parser/18", 11);
    ASSERT_EQ_ERR("parser/19", 6);
    ASSERT_EQ_ERR("parser/20", 0);
    ASSERT_EQ_ERR("parser/21", 10);
    ASSERT_EQ_ERR("parser/22", 1);
    ASSERT_EQ_ERR("parser/23", 1);
    ASSERT_EQ_ERR("parser/24", 1);
    ASSERT_EQ_ERR("parser/25", 5);
    ASSERT_EQ_ERR("parser/26", 1);
    ASSERT_EQ_ERR("parser/27", 10);
    ASSERT_EQ_ERR("parser/28", 0);
    ASSERT_EQ_ERR("parser/29", 0);
    ASSERT_EQ_ERR("parser/30", 0);
    ASSERT_EQ_ERR("parser/31", 0);
    ASSERT_EQ_ERR("parser/32", 0);
    ASSERT_EQ_ERR("parser/33", 0);
    ASSERT_EQ_ERR("parser/34", 1);
    ASSERT_EQ_ERR("parser/35", 0);
    ASSERT_EQ_ERR("parser/36", 0);
    ASSERT_EQ_ERR("parser/37", 1);
    ASSERT_EQ_ERR("parser/38", 0);
    ASSERT_EQ_ERR("parser/39", 0);
    ASSERT_EQ_ERR("parser/40", 6);
    ASSERT_EQ_ERR("parser/41", 3);
    ASSERT_EQ_ERR("parser/42", 1);
    ASSERT_EQ_ERR("parser/43", 0);
    ASSERT_EQ_ERR("parser/44", 1);
    ASSERT_EQ_ERR("parser/45", 1);
    ASSERT_EQ_ERR("parser/46", 1);
    ASSERT_EQ_ERR("parser/47", 1);
    ASSERT_EQ_ERR("parser/48", 4);
    ASSERT_EQ_ERR("parser/49", 1);
    ASSERT_EQ_ERR("parser/50", 0);
    ASSERT_EQ_ERR("parser/51", 1);
    ASSERT_EQ_ERR("parser/52", 5);
    ASSERT_EQ_ERR("parser/53", 0);
    ASSERT_EQ_ERR("parser/54", 2);
    ASSERT_EQ_ERR("parser/55", 0);
    ASSERT_EQ_ERR("parser/56", 1);

    return TEST_RESULT;
}

br_test_result_t test_hir(void) {
    TEST_INIT("hir");
    char* args1[] = {"bearc", "07.br", "-i", "tests/hir"};
    ASSERT_EQ_ERR_FROM_ARGS(args1, 0);
    char* args2[] = {"bearc", "tests/hir/07.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args2, 2);
    char* args3[] = {"bearc", "tests/hir/04.br", "--compile", "--import-path", "."};
    ASSERT_EQ_ERR_FROM_ARGS(args3, 10);
    char* args4[] = {"bearc", "tests/hir/00.br", "--compile", "--import-path", "."};
    ASSERT_EQ_ERR_FROM_ARGS(args4, 2);
    char* args5[] = {"bearc", "-i", "tests/projects/00", "-c", "00.br"};
    ASSERT_EQ_ERR_FROM_ARGSN(args5, 3, 2);
    char* args6[] = {"bearc", "-i", "tests/projects/01", "-c", "00.br"};
    ASSERT_EQ_ERR_FROM_ARGSN(args6, 4, 2);
    char* args7[] = {"bearc", "00.br", "-i", "tests/projects/02"};
    ASSERT_EQ_ERR_FROM_ARGSN(args7, 4, 3);
    char* args8[] = {"bearc", "tests/hir/09.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args8, 2);
    char* args9[] = {"bearc", "tests/hir/10.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args9, 2);
    char* args10[] = {"bearc", "tests/hir/11.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args10, 8);
    char* args11[] = {"bearc", "tests/hir/12.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args11, 3);
    char* args12[] = {"bearc", "tests/hir/06.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args12, 8);
    char* args13[] = {"bearc", "tests/hir/13.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args13, 31);
    char* args14[] = {"bearc", "tests/hir/14.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args14, 5);
    char* args15[] = {"bearc", "tests/hir/15.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args15, 15);
    char* args16[] = {"bearc", "tests/hir/16.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args16, 9);
    char* args17[] = {"bearc", "tests/hir/17.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args17, 5);
    char* args18[] = {"bearc", "tests/hir/18.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args18, 6);
    char* args19[] = {"bearc", "tests/hir/19.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args19, 14);
    char* args20[] = {"bearc", "tests/hir/20.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args20, 21);
    char* args21[] = {"bearc", "tests/hir/21.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args21, 5);
    char* args22[] = {"bearc", "tests/hir/22.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args22, 5);
    char* args23[] = {"bearc", "tests/hir/23.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args23, 4);
    char* args24[] = {"bearc", "tests/hir/24.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args24, 2);
    char* args25[] = {"bearc", "tests/hir/25.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args25, 8);
    char* args26[] = {"bearc", "00.br", "-i", "tests/projects/03"};
    ASSERT_EQ_ERR_FROM_ARGSN(args26, 9, 3);
    char* args27[] = {"bearc", "tests/hir/26.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args27, 2);
    char* args28[] = {"bearc", "tests/hir/27.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args28, 21);
    char* args29[] = {"bearc", "tests/hir/28.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args29, 5);
    char* args30[] = {"bearc", "tests/hir/30.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args30, 6);
    char* args31[] = {"bearc", "tests/hir/31.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args31, 0);
    char* args32[] = {"bearc", "tests/hir/32.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args32, 14);
    char* args33[] = {"bearc", "tests/hir/33.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args33, 10);
    char* args34[] = {"bearc", "tests/hir/34.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args34, 4);
    char* args35[] = {"bearc", "tests/hir/35.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args35, 2);
    char* args36[] = {"bearc", "tests/hir/36.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args36, 3);
    char* args37[] = {"bearc", "tests/hir/37.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args37, 1);
    char* args38[] = {"bearc", "tests/hir/38.br"};
    ASSERT_EQ_ERR_FROM_ARGS(args38, 4);

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
    const char* color = ansi_bold_reset();
    if (succ == tot) {
        color = ansi_bold_green();
    } else if (tot != 0 && ((double)succ / (double)tot >= .9)) {
        color = ansi_bold_yellow();
    } else {
        color = ansi_bold_red();
    }
    printf("  %s%-18s   %s%zu/%zu%s\n", ansi_bold(), res->name, color, succ, tot, ansi_reset());
}
