//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#include "compiler/hir/diagnostic.hpp"
#include "compiler/hir/context.hpp"
#include "compiler/hir/indexing.hpp"
#include "compiler/hir/type.hpp"
#include "utils/ansi_codes.h"
#include "utils/file_io.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <utility>
#include <variant>

namespace hir {

void Diagnostic::print(Context& context, bool print_file) const {
    const src_buffer_t* buf = context.ast(span.file_id).src();
    auto s = Span::retrieve_from_buffer(buf->data, span);
    print_multiline(context, print_file);
    /*
    else {
        print_diagnostic(buf, (char*)s.data(), span.len, span.line, span.col,
                         accent_color_for_type(type), name_for_type(type), message_for_code(code),
                         "", context.compact_diagnostics_enabled());
    }
    */
}

const char* Diagnostic::message_for_code(enum diag_code c) {
    switch (c) {
    case diag_code::count:
        break;
    case diag_code::redefinition:
        return "redefined symbol";
    case diag_code::previous_def_here:
        return "previous definition here";
    case diag_code::no_matching_struct_for_method:
        return "no matching struct for method declaration";
    case diag_code::capitalized_mod:
        return "capitalized module name";
    case diag_code::lowercase_structure:
        return "lowercase structure name";
    case diag_code::invalid_extern_lang:
        return "unkown extern ABI:";
    case diag_code::imported_file_dne:
        return "imported file does not exist";
    case diag_code::cyclical_import:
        return "cyclical file import detected";
    case diag_code::empty_variant:
        return "empty variant not permitted";
    case diag_code::empty_union:
        return "empty union not permitted";
    case diag_code::circular_definition_origin:
        return "circular definition origin here";
    case diag_code::circular_definition:
        return "circular definition; layout cannot be resolved";
    case diag_code::circular_definition_passes_thru:
        return "circular definition passes through this definition";
    case diag_code::value_cannot_be_compt:
        return "cannot resolve value at compile-time";
    case diag_code::cannot_convert_type:
        return "type conversion not possible";
    case diag_code::cannot_resolve_at_compt:
        return "cannot resolve expression at compile-time";
    case diag_code::type_not_defined:
        return "type not defined";
    case diag_code::must_initialize_global_variable:
        return "global variables must be initialized";
    case diag_code::compt_variable_should_be_immutable:
        return "compt variables must be immutable";
    case diag_code::cannot_init_with_non_compt_value:
        return "initializer for 'compt' value must be a compile-time constant";
    case diag_code::declared_here_without_compt:
        return "declared here without 'compt' specifier";
    case diag_code::not_a_compile_time_constant:
        return "not a compile-time constant";
    case diag_code::use_of_undeclared_identifier:
        return "use of undeclared identifier";
    case diag_code::not_declared_in_this_scope:
        return "not declared in this scope";
    case diag_code::replace_with:
        return "replace with";
    case diag_code::remove:
        return "remove";
    case diag_code::invalid_alignas:
        return "invalid alignas alignment width provided";
    case diag_code::alignas_expr_must_be_a_valid_uint_lit:
        return "alignment must be an unsigned integer literal that is a power of 2 and <= 128.\n"
               "      Specified alignments incompatible with your system's architecture will "
               "default\n"
               "      to their standard values. Consult your architecture and LLVM documentation "
               "for\n"
               "      more details.";

    case diag_code::multiple_alignas_on_one_def:
        return "duplicate `alignas` modifiers on one declaration is malformed";
    case diag_code::redundant_compt_qualifier:
        return "redundant `compt` qualifier";
    case diag_code::redundant_static_qualifier:
        return "redundant `static` qualifier";
    case diag_code::compt_generic_structs_not_possible:
        return "compt generic struct initialization is not yet availible";
    case diag_code::this_feature_is_planned:
        return "this feature is planned";
    case diag_code::is_not_a_struct:
        return "is not a struct";
    case diag_code::declared_here:
        return "declared here";
    }
    std::unreachable();
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
    case diag_type::help:
        return "help";
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
    case diag_type::help:
        return ansi_bold_green();
    }
    return "";
}

void Diagnostic::print_line(const auto& printable) const {
    std::cout << "  " << std::setw(width(span.line)) << "" << "  | " << printable << '\n';
}

void Diagnostic::print_line_with_number(HirSize line, const auto& printable) const {
    std::cout << "  " << line << "  | " << printable << '\n';
}

std::string Diagnostic::line(int min_width) const {
    std::stringstream ss;
    ss << "  " << std::setw(std::max(min_width, width((span.line)))) << "" << "  | ";
    return ss.str();
}

std::string Diagnostic::diag(int min_width) const {
    std::stringstream ss;
    ss << "  " << std::setw(std::max(min_width, width((span.line)))) << "" << "  \\";
    return ss.str();
}

std::string Diagnostic::line_with_number(HirSize line, int min_width) const {
    std::stringstream ss;
    ss << "  " << std::setw(std::max(min_width, width((span.line)))) << line << "  | ";
    return ss.str();
}

int Diagnostic::width(HirSize line) {
    int w = 1;
    while (line >= 10) {
        line /= 10;
        ++w;
    }
    return w;
}

// TODO make this return a string for improved render flexibility
void Diagnostic::print_info_value(Context& context, HirSize min_width) const {

    auto import_stack_helper = [&](IdSlice<FileId> files) {
        auto stream_trace = [&](FileId id, int idx) {
            const char* lore = (idx == 0) ? "[!]   cycle origin: " : "|--> which imports: ";
            const char* clr = (idx == 0) ? ansi_bold_cyan() : "";
            return std::stringstream{} << clr << lore << ansi_bold_reset() << context.file_name(id)
                                       << ansi_reset();
        };
        int idx = 0;
        std::cout << '\n';
        for (auto fid = IdIdx<FileId>{files.begin().val() + 1}; fid != files.end(); ++fid) {
            print_line(stream_trace(context.file_id(fid), idx).str());
            idx++;
        }
        print_line(stream_trace(context.file_id(files.first()), idx).str());
    };

    const auto vs = Ovld{
        [&](DiagnosticNoOtherInfo) { std::cout << '\n'; },
        [&](DiagnosticCannotConvertFromTypeToType t) {
            std::cout << accent_color_for_type(type) << "cannot convert value of type `"
                      << type_to_string(context, t.from) << "` to `"
                      << type_to_string(context, t.to) << '`' << ansi_reset() << '\n';
        },
        [&](DiagnosticImportStack import_stack) { import_stack_helper(import_stack.files); },
        [&](DiagnosticSubCode sc) {
            std::cout << accent_color_for_type(type) << message_for_code(sc.sub_code)
                      << ansi_reset() << '\n';
        },
        [](DiagnosticInfoNoPreview) {},
    };
    this->visit(vs);
    if (!holds<DiagnosticNoOtherInfo>() && !context.compact_diagnostics_enabled()) {
        std::cout << line(static_cast<int>(min_width))
                  << '\n'; // extra line for readibility for stacked diags
    }
}

void Diagnostic::print_multiline(Context& context, bool print_file) const {
    const char* file_name = context.file_name(span.file_id);
    auto adjusted_line = span.line + 1;
    auto adjusted_col = span.col + 1;
    const char* accent_color = accent_color_for_type(type);
    std::string complex_message_str{};
    if (has_complex_message()) {
        build_complex_message(context, complex_message_str);
    }
    const char* message
        = has_complex_message() ? complex_message_str.c_str() : message_for_code(code);
    if (context.compact_diagnostics_enabled()) {
        if (print_file) {
            printf("%s%s:%u:%u: ", ansi_bold_reset(), file_name, adjusted_line, adjusted_col);
        }
        printf("%s%s: %s%s%s\n", accent_color, name_for_type(type), ansi_bold_reset(), message,
               ansi_reset());
    } else {
        printf("%s%s: %s%s \n", accent_color, name_for_type(type), ansi_bold_reset(), message);
        if (print_file) {
            printf(" --> %s:%u:%u %s\n", file_name, adjusted_line, adjusted_col, ansi_reset());
        }
    }

    // if it's help, don't reshow the src buffer!
    if (type == diag_type::help || this->holds<DiagnosticInfoNoPreview>()) {
        return;
    }

    const char* span_start = span.as_sv(context).data();
    const char* span_end = span_start + span.len;

    const char* src_buf_span_start = span_start; // start here before adjusting
    size_t src_buf_span_len = span.len;
    const char* src_buf_start = context.ast(span.file_id).buffer();
    // find start of src
    while (src_buf_span_start > src_buf_start && *src_buf_span_start != '\n') {
        --src_buf_span_start;
        ++src_buf_span_len;
    }
    // as to not having a leading newline
    if (src_buf_span_start[0] == '\n') {
        ++src_buf_span_start;
    }

    // as to not have a trailing newline
    if (src_buf_span_start[src_buf_span_len - 1] == '\n') {
        assert(src_buf_span_len != 0);
        --src_buf_span_len;
    }

    // end (keep bumping end till newline or end of buffer)
    while (src_buf_span_start[src_buf_span_len] != '\0'
           && src_buf_span_start[src_buf_span_len] != '\n') {
        ++src_buf_span_len;
    }

    // find max line (for line number sidebar width purposes)
    HirSize max_line = adjusted_line; // start here
    for (size_t i = 0; i < src_buf_span_len; i++) {
        if (src_buf_span_start[i] == '\n') {
            ++max_line;
        }
    }

    // max line width in terms of characters in the line number string
    const int min_width = width(max_line);

    // print body now
    static constexpr size_t BUF_DEF_SIZE = 2048;
    std::string buf;
    buf.reserve(BUF_DEF_SIZE);
    std::string_view full_src_span{src_buf_span_start, src_buf_span_len};

    HirSize curr_line = adjusted_line; // start here
    bool do_line_num = true;
    bool do_faux_line = false;
    static constexpr HirSize MAX_LINE_LEN = 100;

    HirSize curr_len = 0;
    HirSize prev_len = 0;

    HirSize issue_line = 0;
    HirSize issue_len = 0;
    bool has_faux_lines = false;

    if (!context.compact_diagnostics_enabled()) {
        std::cout << line(min_width) << '\n';
    }

    for (HirSize i = 0; i < full_src_span.size(); i++) {

        auto maybe_accent_color = [&] {
            // if in span
            if (full_src_span.data() + i >= span_start && full_src_span.data() + i < span_end) {
                return accent_color;
            }
            return "";
        };

        // no line num
        auto line_without_num
            = [&] -> std::string { return ansi_reset() + line(min_width) + maybe_accent_color(); };
        // w line num
        auto line_with = [&](HirSize line_num) -> std::string {
            return ansi_reset() + line_with_number(line_num, min_width) + maybe_accent_color();
        };

        const char c = full_src_span[i];

        // handle need to line num
        if (do_line_num) {
            buf += line_with(curr_line);
            do_line_num = false;
        }
        if (c == '\n') {
            do_line_num = true;
            prev_len = curr_len;
            curr_len = 0;
            ++curr_line;
        } else if (curr_len >= MAX_LINE_LEN - 1) {
            do_faux_line = true;
            has_faux_lines = true;
            prev_len = curr_len;
            curr_len = 0;
            ++curr_line;
        }

        // start colored span
        if (full_src_span.data() + i == span_start) {
            buf += accent_color;
            issue_line = curr_line;
            issue_len = curr_len;
        } else if (full_src_span.data() + i == span_end) {
            buf += ansi_reset();
        }

        buf += c;

        if (do_faux_line) {
            // make a full word before line break;
            std::string tmp;
            tmp.reserve(MAX_LINE_LEN);
            while (buf.size() >= 1 && buf.at(buf.size() - 1) != ' ') {
                const char c = buf.at(buf.size() - 1);
                buf.pop_back();
                tmp.push_back(c);
            }
            buf += '\n';
            buf += line_without_num();
            buf += "        "; // 8 spaces
            for (auto iter = tmp.rbegin(); iter != tmp.rend(); iter++) {
                buf += (*iter);
            }
            do_faux_line = false;
        }

        bool single_line_done
            = !has_faux_lines && (adjusted_line == max_line) && (i == full_src_span.size() - 1);
        if (single_line_done) {
            if (c != '\n') {
                buf += '\n';
            }
            buf += line_without_num();
            for (HirSize k = 0; k < issue_len; k++) {
                buf += ' ';
            }
            buf += accent_color;
            for (HirSize k = 0; k <= span.len - 1; k++) {
                buf += '^';
            }
        }

        ++curr_len;
    }
    buf += ansi_reset();

    std::cout << buf;

    // if only one line
    if (curr_line == adjusted_line) {
        if (issue_len > MAX_LINE_LEN - 40) {
            std::string pre_info_buf;
            pre_info_buf.reserve(MAX_LINE_LEN);
            for (HirSize i = 0; i < issue_len; i++) {
                pre_info_buf += ' ';
            }
            std::cout << '\n' << line(min_width) << pre_info_buf;
        } else {
            std::cout << ' ';
        }

    } else {
        std::cout << '\n' << line(min_width);
    }
    // else if truly multiline

    print_info_value(context, min_width);
}

bool Diagnostic::has_complex_message() const {
    return !std::holds_alternative<DiagnosticNoOtherInfo>(message_value);
}

void Diagnostic::build_complex_message(const Context& ctx, std::string& str) const {
    str.reserve(64); // decent size
    auto vs = Ovld{[](DiagnosticNoOtherInfo) {},
                   [&](DiagnosticIdentifierAfterMessage d) {
                       str += message_for_code(code);
                       str += " `";
                       str += accent_color_for_type(type);
                       for (auto sidx = d.sid_slice.begin(); sidx != d.sid_slice.end(); sidx++) {
                           str += ctx.symbol_id_to_cstr(ctx.symbol_id(sidx));
                           if (sidx != d.sid_slice.last_elem()) {
                               str += "..";
                           }
                       }
                       str += ansi_bold_reset();
                       str += '`';
                   },
                   [&](DiagnosticIdentifierBeforeMessage d) {
                       str += '`';
                       str += accent_color_for_type(type);
                       for (auto sidx = d.sid_slice.begin(); sidx != d.sid_slice.end(); sidx++) {
                           str += ctx.symbol_id_to_cstr(ctx.symbol_id(sidx));
                           if (sidx != d.sid_slice.last_elem()) {
                               str += "..";
                           }
                       }
                       str += ansi_bold_reset();
                       str += "` ";
                       str += message_for_code(code);
                   },
                   [&](DiagnosticSymbolAfterMessage d) {
                       str += message_for_code(code);
                       str += " `";
                       str += accent_color_for_type(type);
                       str += ctx.symbol_id_to_cstr(d.sid);
                       str += ansi_bold_reset();
                       str += '`';
                   },
                   [&](DiagnosticSymbolAfterMessageNoQuotes d) {
                       str += message_for_code(code);
                       str += ' ';
                       str += ctx.symbol_id_to_cstr(d.sid);
                   }};
    std::visit(vs, message_value);
}

} // namespace hir
