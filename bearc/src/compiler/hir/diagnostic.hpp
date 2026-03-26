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
    previous_def_here,
    no_matching_struct_for_method,
    capitalized_mod,
    lowercase_structure,
    invalid_extern_lang,
    cyclical_import,
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
    cannot_init_with_non_compt_value,
    declared_here_without_compt,
    not_a_compile_time_constant,
    use_of_undeclared_identifier,
    not_declared_in_this_scope,
    replace_with,
    remove,
    invalid_alignas,
    alignas_expr_must_be_a_valid_uint_lit,
    multiple_alignas_on_one_def,
    redundant_compt_qualifier,
    redundant_static_qualifier,
    compt_generic_structs_not_possible,
    this_feature_is_planned,
    is_not_a_struct,
    declared_here,
    struct_field_not_initialized,
    struct_fields_not_initialized,
    cannot_convert_expression_to_type,
    field_initializer_does_not_match_field,
    too_many_initializers_given_for_struct_init,
    compt_values_cannot_be_moved,
    var_cannot_be_part_of_a_scoped_identifier,
    compt_variable_should_have_an_explicit_type,
    count, // this must be last,
};
enum class diag_type : uint8_t { error, warning, note, help };

struct DiagnosticNoOtherInfo {};

struct DiagnosticIdentifierAfterMessage {
    IdSlice<SymbolId> sid_slice;
};

struct DiagnosticIdentifierBeforeMessage {
    IdSlice<SymbolId> sid_slice;
};

struct DiagnosticSymbolBeforeMessage {
    SymbolId sid;
};

struct DiagnosticSymbolAfterMessage {
    SymbolId sid;
};

struct DiagnosticSymbolAfterMessageNoQuotes {
    SymbolId sid;
};

struct DiagnosticCannotConvertToType {
    TypeId tid;
};

using DiagnosticMessageValue
    = std::variant<DiagnosticNoOtherInfo, DiagnosticIdentifierAfterMessage,
                   DiagnosticSymbolAfterMessage, DiagnosticSymbolAfterMessageNoQuotes,
                   DiagnosticIdentifierBeforeMessage, DiagnosticCannotConvertToType,
                   DiagnosticSymbolBeforeMessage>;

struct DiagnosticImportStack {
    IdSlice<FileId> files;
};

struct DiagnosticInfoNoPreview {};

struct DiagnosticCannotConvertFromTypeToType {
    TypeId from;
    TypeId to;
};

struct DiagnosticSubCode {
    diag_code sub_code;
};

using DiagnosticInfoValue = std::variant<DiagnosticNoOtherInfo, DiagnosticImportStack,
                                         DiagnosticCannotConvertFromTypeToType, DiagnosticSubCode,
                                         DiagnosticInfoNoPreview>;

struct Diagnostic : NodeWithVariantValue<Diagnostic> {

    using id_type = DiagnosticId;
    using value_type = DiagnosticInfoValue;
    DiagnosticMessageValue message_value = DiagnosticNoOtherInfo{};
    DiagnosticInfoValue value = DiagnosticNoOtherInfo{};
    Span span;
    OptId<DiagnosticId> next;
    diag_code code;
    diag_type type;
    void print(Context& context, bool print_file) const;
    Diagnostic(Span span, enum diag_code code, enum diag_type type,
               OptId<DiagnosticId> next = OptId<DiagnosticId>{})
        : span(span), code(code), type(type), next(next), value(DiagnosticNoOtherInfo{}) {}
    Diagnostic(Span span, enum diag_code code, enum diag_type type, DiagnosticInfoValue value,
               OptId<DiagnosticId> next = OptId<DiagnosticId>{})
        : span(span), code(code), type(type), next(next), value(value) {}
    Diagnostic(Span span, enum diag_code code, enum diag_type type,
               DiagnosticMessageValue message_value, DiagnosticInfoValue value,
               OptId<DiagnosticId> next = OptId<DiagnosticId>{})
        : span(span), code(code), type(type), next(next), message_value(message_value),
          value(value) {}
    void set_next(DiagnosticId next) { this->next = next; }

  private:
    bool has_complex_message() const;
    void build_complex_message(const Context& ctx, std::string& message_str) const;
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
