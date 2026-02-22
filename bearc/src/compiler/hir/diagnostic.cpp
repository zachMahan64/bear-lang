//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/diagnostic.hpp"
#include "compiler/diagnostics/error_list.h"
#include "compiler/hir/context.hpp"
#include "utils/ansi_codes.h"
#include "utils/file_io.h"
#include <iomanip>
#include <iostream>

namespace hir {

void Diagnostic::print(const Context& context) const {
    const src_buffer_t* buf = context.c_ast(span.file_id).src();
    auto s = Span::retrieve_from_buffer(buf->data, span);
    print_diagnostic(buf, (char*)s.data(), span.len, span.line, span.col,
                     accent_color_for_type(type), name_for_type(type), message_for_code(code), "");
    // TODO handle DiagnosticValue
}

const char* Diagnostic::message_for_code(enum diag_code c) {
    switch (c) {
    case diag_code::redefinition:
        return "redefined symbol";
    case diag_code::original_def_here:
        return "redefined symbol originally defined here";
    case diag_code::no_matching_struct_for_method:
        return "no matching struct for method declaration";
        break;
    case diag_code::capitalized_mod:
        return "capitalized module name";
        break;
    case diag_code::lowercase_structure:
        return "lowercase structure name";
        break;
    case diag_code::invalid_extern_lang:
        return "invalid language target specified for external linkage";
        break;
    }
    return "";
}
const char* Diagnostic::name_for_type(enum diag_type t) {
    switch (t) {
    case diag_type::error:
        return "error";
    case diag_type::warning:
        return "warning";
    case diag_type::note:
        return "note";
    }
    return "diagnostic";
}
const char* Diagnostic::accent_color_for_type(enum diag_type t) {
    switch (t) {
    case diag_type::error:
        return ansi_bold_red();
    case diag_type::warning:
    case diag_type::note:
        return ansi_bold_yellow();
    }
    return "";
}

} // namespace hir
