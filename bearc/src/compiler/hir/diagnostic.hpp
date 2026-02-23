//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_DIAGNOSTIC_HPP
#define COMPILER_HIR_DIAGNOSTIC_HPP

#include "compiler/hir/indexing.hpp"
#include "compiler/hir/span.hpp"
#include <cstdint>
#include <variant>
namespace hir {

class Context;
enum class diag_code : uint8_t {
    imported_file_dne,
    redefinition,
    original_def_here,
    no_matching_struct_for_method,
    capitalized_mod,
    lowercase_structure,
    invalid_extern_lang,
    cyclical_import,
};
enum class diag_type : uint8_t { error, warning, note };

struct DiagnosticNoOtherInfo {};
struct DiagnosticImportStack {
    IdSlice<FileId> files;
};

using DiagnosticValue = std::variant<DiagnosticNoOtherInfo, DiagnosticImportStack>;

struct Diagnostic {

    using id_type = DiagnosticId;
    DiagnosticValue value;
    Span span;
    OptId<DiagnosticId> next;
    diag_code code;
    diag_type type;
    void print(const Context& context) const;
    Diagnostic(Span span, enum diag_code code, enum diag_type type,
               OptId<DiagnosticId> next = OptId<DiagnosticId>{})
        : span(span), code(code), type(type), next(next), value(DiagnosticNoOtherInfo{}) {}
    Diagnostic(Span span, enum diag_code code, enum diag_type type, DiagnosticValue value,
               OptId<DiagnosticId> next = OptId<DiagnosticId>{})
        : span(span), code(code), type(type), next(next), value(value) {}
    void set_next(DiagnosticId next) { this->next = next; }

  private:
    static const char* message_for_code(enum diag_code c);
    static const char* name_for_type(enum diag_type t);
    static const char* accent_color_for_type(enum diag_type t);
    void print_info_value(const Context& context) const;
};

} // namespace hir

#endif
