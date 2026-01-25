//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/span/span.h"
#include "utils/file_io.h"
#include "utils/string_view.h"
#include <stddef.h>

span_t span_normalize_src_view_from_tkn(spannable_file_t file, token_t* tkn) {
    return span_normalize_src_view(file, tkn->start, tkn->len);
}

span_t span_normalize_src_view(spannable_file_t file, const char* start, size_t len) {
    const char* data = file.src_view->data;
    span_t span = {.file_id = file.file_id, .start = start - data, .end = start + len - data};
    return span;
}

string_view_t span_retrieve_from_buffer(const char* data, span_t span) {
    string_view_t view = {.start = data + span.start, .len = span.end - span.start};
    return view;
}
