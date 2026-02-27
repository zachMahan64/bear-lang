//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/span.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/indexing.hpp"
#include <stddef.h>
#include <string_view>

namespace hir {
Span::Span(FileId file_id, const char* src, const token_t* tkn)
    : start(tkn->start - src), len(tkn->len), file_id(file_id), line(tkn->loc.line),
      col(tkn->loc.col) {}

Span::Span(FileId file_id, const char* src, const token_t* first, const token_t* last)
    : start(first->start - src), file_id(file_id), len(last->start - src + last->len),
      line(first->loc.line), col(first->loc.col) {}

std::string_view Span::retrieve_from_buffer(const char* data, Span span) {
    return std::string_view(data + span.start, span.len);
}

[[nodiscard]] std::string_view Span::as_sv(const Context& context) const {
    return retrieve_from_buffer(context.c_ast(file_id).buffer(), *this);
}

} // namespace hir
