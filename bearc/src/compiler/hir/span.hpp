//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_SPAN_SPAN
#define COMPILER_SPAN_SPAN

#include "compiler/hir/indexing.hpp"
#include "compiler/token.h"
#include <stdbool.h>
#include <stddef.h>
#include <string_view>

namespace hir {

class Context;

class Span {
    Span(HirSize start, HirSize len, FileId file_id, HirSize line, HirSize col) noexcept
        : start(start), len(len), file_id(file_id), line(line), col(col) {};

  public:
    HirSize start;
    HirSize len;
    FileId file_id;
    HirSize line;
    HirSize col;
    /// constructs an hir::Span from an existing FileId and none-owned ptrs to a src buffer and a
    /// token_t
    Span(FileId file_id, const char* src, const token_t* tkn);
    Span(FileId file_id, const char* src, const token_t* first, const token_t* last);
    Span(const Context& ctx, FileId file_id, const token_t* first, const token_t* last);
    [[nodiscard]] static std::string_view retrieve_from_buffer(const char* data, Span span);
    [[nodiscard]] std::string_view as_sv(const Context& context) const;
    static Span generated();
};

} // namespace hir

#endif
