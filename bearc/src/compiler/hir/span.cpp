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
Span::Span(HirSize start, HirSize len, FileId file_id, HirSize line) noexcept
    : start(start), len(len), file_id(file_id), line(line) {}

Span::Span(FileId file_id, const char* src, token_t* tkn)
    : start(tkn->start - src), len(tkn->len), file_id(file_id), line(tkn->loc.line) {}

Span::Span(FileId file_id, const char* src, token_t* first, token_t* last)
    : start(first->start - src), len(last->start - src + last->len), file_id(file_id),
      line(first->loc.line) {}

std::string_view Span::retrieve_from_buffer(const char* data, Span span) {
    return std::string_view(data + span.start, span.len);
}

} // namespace hir
