//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_TOP_LEVEL_DEF_VISITOR_HPP
#define COMPILER_TOP_LEVEL_DEF_VISITOR_HPP

#include "compiler/hir/context.hpp"
namespace hir {

class TopLevelVisitor {
    Context& context;

  public:
    TopLevelVisitor(Context& context) : context{context} {}
};

} // namespace hir

#endif
