//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_SPAN_SPAN
#define COMPILER_SPAN_SPAN

#include "compiler/hir/indexing.hpp"
#include "compiler/token.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string_view>

namespace hir {

/// trivially copyable src span, always pass by value
class Span {
    Span(HirSize start, HirSize len, FileId file_id, HirSize line) noexcept;

  public:
    HirSize start;
    HirSize len;
    FileId file_id;
    // col can be found be backtracking to sof or last \n
    HirSize line;
    /// constructs an hir::Span from an existing FileId and none-owned ptrs to a src buffer and a
    /// token_t
    Span(FileId file_id, const char* src, token_t* tkn);
    Span(FileId file_id, const char* src, token_t* first, token_t* last);
    [[nodiscard]] static std::string_view retrieve_from_buffer(const char* data, Span span);
};

} // namespace hir

#endif
