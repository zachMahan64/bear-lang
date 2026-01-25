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
#include "utils/file_io.h"
#include "utils/string_view.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct {
    uint32_t start;
    uint32_t end;
    hir_file_id_t file_id;
} span_t;

span_t span_normalize_src_view(src_buffer_t* src_buffer, const char* start, size_t len);

string_view_t span_retrieve(const char* data, span_t span);

#endif
