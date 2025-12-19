//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/ast/type.h"
#include "compiler/parser/parse_token_slice.h"

ast_type_t* parse_type_with_leading_mut(parser_t* p);

ast_type_t* parse_type_with_leading_id(parser_t* p, token_ptr_slice_t id_slice);
