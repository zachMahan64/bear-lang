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
    : start(first->start - src), file_id(file_id), len((last->start + last->len) - first->start),
      line(first->loc.line), col(first->loc.col) {}

std::string_view Span::retrieve_from_buffer(const char* data, Span span) {
    return std::string_view(data + span.start, span.len);
}

[[nodiscard]] std::string_view Span::as_sv(const Context& context) const {
    return retrieve_from_buffer(context.ast(file_id).buffer(), *this);
}

Span::Span(const Context& ctx, FileId file_id, const token_t* first, const token_t* last)
    : Span(file_id, ctx.ast(file_id).buffer(), first, last) {}
Span Span::generated() { return Span{0, 0, FileId{HIR_ID_NONE}, 0, 0}; }

Span Span::combine(Span span1, Span span2) {
    return Span(span1.start, span2.start - span1.start + span2.len, span1.file_id, span1.line,
                span1.col);
}

} // namespace hir
