//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/diagnostic.hpp"
#include "compiler/diagnostics/error_list.h"
#include "compiler/hir/context.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/type.hpp"
#include "utils/ansi_codes.h"
#include "utils/file_io.h"
#include <cmath>
#include <iostream>
#include <sstream>

namespace hir {

void Diagnostic::print(const Context& context) const {
    const src_buffer_t* buf = context.ast(span.file_id).src();
    auto s = Span::retrieve_from_buffer(buf->data, span);
    print_diagnostic(buf, (char*)s.data(), span.len, span.line, span.col,
                     accent_color_for_type(type), name_for_type(type), message_for_code(code), "");
    print_info_value(context);
}

const char* Diagnostic::message_for_code(enum diag_code c) {
    switch (c) {
    case diag_code::redefinition:
        return "redefined symbol";
    case diag_code::original_def_here:
        return "redefined symbol originally defined here";
    case diag_code::no_matching_struct_for_method:
        return "no matching struct for method declaration";
    case diag_code::capitalized_mod:
        return "capitalized module name";
    case diag_code::lowercase_structure:
        return "lowercase structure name";
    case diag_code::invalid_extern_lang:
        return "invalid language target specified for external linkage";
    case diag_code::imported_file_dne:
        return "imported file does not exist";
    case diag_code::cyclical_import:
        return "circular file imports are not permitted";
    case diag_code::empty_struct:
        return "empty struct not permitted";
    case diag_code::empty_variant:
        return "empty variant not permitted";
    case diag_code::empty_union:
        return "empty union not permitted";
    case diag_code::circular_definition_origin:
        return "circular definition origin here";
        break;
    case diag_code::circular_definition:
        return "circular definition; layout cannot be resolved";
        break;
    case diag_code::circular_definition_passes_thru:
        return "circular definition passes through this definition";
        break;
    case diag_code::value_cannot_be_compt:
        return "cannot resolve value at compile-time";
        break;
    case diag_code::cannot_convert_to_some_builtin_type:
        return "type mismatch; cannot assign to variable";
        break;
    case diag_code::cannot_resolve_at_compt:
        return "cannot resolve expression at compile-time";
        break;
    case diag_code::type_not_defined:
        return "type not defined";
        break;
    case diag_code::must_initialize_global_variable:
        return "global variables must be initialized";
        break;
    case diag_code::compt_variable_should_be_immutable:
        return "type of variable declared as 'compt' should be immutable";
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
        return ansi_bold_yellow();
    case diag_type::note:
        return ansi_bold_cyan();
    }
    return "";
}

void Diagnostic::print_info_value(Context& context) const {
    auto line = [&](const auto& printable) {
        std::cout << "  " << std::setw(static_cast<int>(log10(span.line + 1)) + 1) << "" << "  | "
                  << printable << '\n';
    };

    auto import_stack_helper = [&](IdSlice<FileId> files) {
        auto stream_trace = [&](FileId id, int idx) {
            const char* lore = (idx == 0) ? "[!]   cycle origin: " : "|--> which imports: ";
            const char* clr = (idx == 0) ? ansi_bold_cyan() : "";
            return std::stringstream{} << clr << lore << ansi_bold_white() << context.file_name(id)
                                       << ansi_reset();
        };
        int idx = 0;
        for (auto fid = IdIdx<FileId>{files.begin().val() + 1}; fid != files.end(); ++fid) {
            line(stream_trace(context.file_id(fid), idx).str());
            idx++;
        }
        line(stream_trace(context.file_id(files.first()), idx).str());
    };

    const auto vs = Ovld{
        [&](DiagnosticNoOtherInfo) { line(""); },
        [&](DiagnosticCannotConvertToBuiltinType t) {
            line((std::stringstream{} << "unable to convert to type: "
                                      << builtin_type_to_cstr(t.type))
                     .str());
        },
        [&](DiagnosticImportStack import_stack) {
            import_stack_helper(import_stack.files);
            /*
            for (auto fid = import_stack.files.first(); fid != import_stack.files.end(); ++fid) {
                std::cout << "imported in: " << ansi_bold_white()
                          << context.file_name(context.file_id_idx_to_id(fid)) << ansi_reset()
                          << '\n';
            }
            */
        },
    };
    this->visit(vs);
}

} // namespace hir
