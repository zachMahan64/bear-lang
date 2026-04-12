//     /                              /
//    /                              /
//   /_____  _____  _____  _____    /  _____   _  _  _____
//  /     / /____  /____/ /____/   /  /____/  /\  / /  __
// /_____/ /____  /    / /   \    /  /    /  /  \/ /____/
// Copyright (C) 2025-2026 Zachary Mahan
// Licensed under the GNU GPL v3. See LICENSE for details.

#ifndef COMPILER_HIR_DIAGNOSTIC_HPP
#define COMPILER_HIR_DIAGNOSTIC_HPP

#include "compiler/hir/exec_ops.hpp"
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
    use_of_undeclared_mod,
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
    cannot_convert_value_of_type,
    cannot_assign_to_compt_constant,
    is_operator_requires_run_time_values,
    cannot_cast_expr_to_type,
    guaranteed_narrowing_of_compt_value,
    invalid_operand_for_binary_expression,
    type_is_not_resolvable_at_compt,
    is_declared_hid,
    a_compt_variable_should_be_explicitly_initialized,
    even_non_compt_top_levels_need_compt_init,
    incompatible_types_for_binary_expression,
    incompatible_types_for_binary_operator,
    dividing_by_zero_at_compt_is_illegal,
    invalid_cast,
    parentheses_should_be_used_for_chained_casts,
    operator_not_viable_at_compt,
    immutable_value_is_not_assignable,
    value_is_not_contextually_convertible_to,
    invalid_operand_for_unary_expression,
    mismatched_types_in_list_literal,
    value_is_of_type,
    is_of_type,
    array_cannot_have_size_zero,
    cannot_infer_type_at_compt,
    static_assertion_failed,

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

struct DiagnosticIdentifierBeforeMessageAndTypeAfter {
    IdSlice<SymbolId> sid_slice;
    TypeId tid;
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

struct DiagnosticTypeAfterMessage {
    TypeId tid;
};

struct DiagnosticTypeToType {
    TypeId from;
    TypeId to;
};

struct DiagnosticTypeAndType {
    TypeId lhs_tid;
    TypeId rhs_tid;
};

struct DiagnosticTypeAndTypeForBinaryOp {
    TypeId lhs_tid;
    TypeId rhs_tid;
    binary_op op;
};

using DiagnosticMessageValue
    = std::variant<DiagnosticNoOtherInfo, DiagnosticIdentifierAfterMessage,
                   DiagnosticSymbolAfterMessage, DiagnosticSymbolAfterMessageNoQuotes,
                   DiagnosticIdentifierBeforeMessage, DiagnosticTypeAfterMessage,
                   DiagnosticSymbolBeforeMessage, DiagnosticTypeToType, DiagnosticTypeAndType,
                   DiagnosticTypeAndTypeForBinaryOp, DiagnosticIdentifierBeforeMessageAndTypeAfter>;

struct DiagnosticImportStack {
    IdSlice<FileId> files;
};

struct DiagnosticInfoNoPreview {};

struct DiagnosticSubCode {
    diag_code sub_code;
};

using DiagnosticInfoValue
    = std::variant<DiagnosticNoOtherInfo, DiagnosticImportStack, DiagnosticTypeToType,
                   DiagnosticSubCode, DiagnosticInfoNoPreview>;

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
        : value(DiagnosticNoOtherInfo{}), span(span), next(next), code(code), type(type) {}
    Diagnostic(Span span, enum diag_code code, enum diag_type type, DiagnosticInfoValue value,
               OptId<DiagnosticId> next = OptId<DiagnosticId>{})
        : value(value), span(span), next(next), code(code), type(type) {}
    Diagnostic(Span span, enum diag_code code, enum diag_type type,
               DiagnosticMessageValue message_value, DiagnosticInfoValue value,
               OptId<DiagnosticId> next = OptId<DiagnosticId>{})
        : message_value(message_value), value(value), span(span), next(next), code(code),
          type(type) {}
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
