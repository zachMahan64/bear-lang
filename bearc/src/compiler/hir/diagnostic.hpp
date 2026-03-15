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
#include "compiler/hir/type.hpp"
#include "compiler/hir/variant_helpers.hpp"
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
    empty_struct,
    empty_variant,
    empty_union,
    circular_definition,
    circular_definition_passes_thru,
    circular_definition_origin,
    value_cannot_be_compt,
    cannot_convert_type,
    cannot_resolve_at_compt,
    type_not_defined,
    compt_variable_should_be_immutable,
    must_initialize_global_variable,
    variable_not_declared_compt,
    count, // this must be last,
};
enum class diag_type : uint8_t { error, warning, note };

struct DiagnosticNoOtherInfo {};
struct DiagnosticImportStack {
    IdSlice<FileId> files;
};

struct DiagnosticCannotConvertFromTypeToType {
    TypeId from;
    TypeId to;
};

using DiagnosticValue = std::variant<DiagnosticNoOtherInfo, DiagnosticImportStack,
                                     DiagnosticCannotConvertFromTypeToType>;

struct Diagnostic : NodeWithVariantValue<Diagnostic> {

    using id_type = DiagnosticId;
    using value_type = DiagnosticValue;
    DiagnosticValue value;
    Span span;
    OptId<DiagnosticId> next;
    diag_code code;
    diag_type type;
    void print(Context& context, bool print_file) const;
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
    void print_info_value(Context& context, HirSize min_width) const;
    void print_multiline(Context& context, bool print_file) const;
    void print_line(const auto& printable) const;
    void print_line_with_number(HirSize line, const auto& printable) const;
    [[nodiscard]] std::string line(int min_width) const;
    [[nodiscard]] std::string diag(int min_width) const;
    [[nodiscard]] std::string line_with_number(HirSize line, int min_width) const;
    static int width(HirSize line);
};

} // namespace hir

#endif
