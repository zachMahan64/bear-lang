//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_SPAN_SPAN
#define COMPILER_SPAN_SPAN

#include "compiler/hir/indexing.h"
#include "compiler/token.h"
#include "utils/file_io.h"
#include "utils/string_view.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/// trivially copyable src span, always pass by value
typedef struct {
    uint32_t start;
    uint32_t end;
    hir_file_id_t file_id;
    // col can be found be backtracking to sof or last \n
    uint32_t line;
} span_t;

/// does not own the src_view
typedef struct {
    const src_buffer_t* const src_view;
    hir_file_id_t file_id;
} spannable_file_t;

span_t span_normalize_src_view_from_tkn(spannable_file_t file, token_t* tkn);

span_t span_normalize_src_view(spannable_file_t file, const char* start, size_t len);

string_view_t span_retrieve_from_buffer(const char* data, span_t span);

#endif
