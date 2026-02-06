//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/exec_ops.hpp"
#include "compiler/token.h"
#include <assert.h>

namespace hir {
binary_op token_to_binary_op(token_t* tkn) {
    switch (tkn->type) {
        // TODO
    default:
        assert(false && "invalid binary op token\n");
        break;
    }
}
unary_op token_to_unary_op(token_t* tkn) {
    switch (tkn->type) {
        // TODO
    default:
        assert(false && "invalid unary op token\n");
        break;
    }
}
} // namespace hir
