//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/span.hpp"
#include "compiler/hir/indexing.hpp"
#include <stddef.h>
#include <string_view>

namespace hir {
Span::Span(FileId file_id, const char* src, token_t* tkn)
    : start(tkn->start - src), len(tkn->len), file_id(file_id), line(tkn->loc.line),
      col(tkn->loc.col) {}

Span::Span(FileId file_id, const char* src, token_t* first, token_t* last)
    : start(first->start - src), file_id(file_id), len(last->start - src + last->len),
      line(first->loc.line), col(first->loc.col) {}

std::string_view Span::retrieve_from_buffer(const char* data, Span span) {
    return std::string_view(data + span.start, span.len);
}

} // namespace hir
