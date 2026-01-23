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
span_t span_normalize_src_view(src_buffer_t* src_buffer, const char* start, size_t len) {
    const char* data = src_buffer->data;
    span_t span = {
        .src_file_name = src_buffer->file_name, .start = start - data, .end = start + len - data};
    return span;
}

string_view_t span_retrieve(const char* data, span_t span) {
    string_view_t view = {.start = data + span.start, .len = span.end - span.start};
    return view;
}
