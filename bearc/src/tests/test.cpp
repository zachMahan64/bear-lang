//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "tests/test.h"
#include "compiler/hir/context_database.hpp"
#include "compiler/hir/exec.hpp"
#include <string>
#include <vector>

using SymSlice = std::vector<std::string>;
using namespace hir;

extern "C" {

br_test_result_t test_context_db(void) {
    TEST_INIT("context database");

    auto econst
        = [&br_test_result](ContextDatabase::DefQueryResult def, const ContextDatabase& db) {
              TEST_ASSERT(def.variable.has_value());

              auto eid = def.variable.value().as<DefVariable>().compt_value;

              TEST_ASSERT(def.variable.value().as<DefVariable>().compt_value.has_value());

              auto exec = db.exec(def.variable.value().as<DefVariable>().compt_value.as_id());

              TEST_ASSERT(exec.holds<ExecConst>());
              return exec;
          };

    const char* args0[] = {"bearc", "tests/hir/28.br"};
    ContextDatabase db28{sizeof(args0) / sizeof(char*), args0};

    auto def0 = db28.query_def({"e"});
    auto exec0 = econst(def0, db28);
    TEST_ASSERT_EQ(exec0.as<ExecConst>().as<i32>(), 0x10);

    auto def1 = db28.query_def({"b1"});
    auto exec1 = econst(def1, db28);
    TEST_ASSERT_EQ(exec1.as<ExecConst>().as<u8>(), (0x11 | 0x10));

    auto def2 = db28.query_def({"h2"});
    auto exec2 = econst(def2, db28);
    TEST_ASSERT_EQ(exec2.as<ExecConst>().as<usize>(), (0x1111 ^ 0x1001));

    return TEST_RESULT;
}

} // extern "C"
