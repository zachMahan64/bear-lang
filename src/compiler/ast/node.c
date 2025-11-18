// BearLang
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE.md for details.

#include "compiler/ast/node.h"
#include <assert.h>
#include <stddef.h>

void ast_node_add_child(ast_node_t* node, size_t idx, ast_node_t* child) {
    assert(idx < node->child_count &&
           "[ERROR|ast_node_add_child] out-of-bounds when adding adding child");
    node->children[idx] = child;
}
