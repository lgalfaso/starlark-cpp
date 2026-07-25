// Copyright 2026 Lucas Mirelmann

#ifndef ERRORS_RUNTIME_ERROR_MESSAGES_HPP_
#define ERRORS_RUNTIME_ERROR_MESSAGES_HPP_

#include <string>
#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace error_messages {

std::string error_not_callable(std::string_view type);
std::string error_unpackable(std::string_view type);
std::string error_unhashable_key(std::string_view type, std::string_view key_type);
std::string error_unhashable_value(std::string_view type, std::string_view value_type);
std::string error_empty_dictionary(std::string_view fn_name);
std::string error_empty_set(std::string_view fn_name);
std::string error_uniterable(std::string_view type);
std::string error_argument_uniterable(std::string_view type);
std::string error_unsubscriptable(std::string_view type);
std::string error_no_item_assignment(std::string_view type);
std::string error_incomparable(std::string_view op, std::string_view type1, std::string_view type2);
std::string error_index_out_of_range(std::string_view type);
std::string error_index_integer_or_slice(std::string_view type, std::string_view actual);
std::string error_index_integer_on_a_slice(std::string_view actual);
std::string error_item_not_in_collection(std::string_view type, std::string_view fn_name);

std::string error_bad_operand_unary(std::string_view op, std::string_view type);
std::string error_bad_operand_binary(std::string_view op, std::string_view type1, std::string_view type2);
std::string error_no_method(std::string_view type, std::string_view method);
std::string error_no_attribute(std::string_view type, std::string_view attribute);
std::string error_no_attribute(std::string_view type, std::string_view attribute, std::string_view suggestion);
std::string error_read_only_attribute(std::string_view type, std::string_view attribute);

std::string error_convert(std::string_view from, std::string_view to);
std::string error_convert_string(std::string_view to, std::string_view string_value);
std::string_view error_convert_float_infinity_to_integer();
std::string_view error_convert_float_nan_to_integer();
std::string error_interpreted_as_integer(std::string_view type);
std::string error_argument_interpreted_as_integer(std::string_view argument_name, std::string_view type);
std::string error_argument_interpreted_as_string(std::string_view argument_name, std::string_view type);

std::string error_dictionary_key_not_found(std::string_view key);
std::string error_dictionary_update_sequence(int64_t position, int64_t actual, int64_t expected);

std::string_view error_byte_in_range();
std::string_view error_bytes_in_range();
std::string_view error_unicode_in_range();

std::string_view error_negative_shift();
std::string_view error_division_by_zero();
std::string error_overflow(std::string_view from, std::string_view to);
std::string_view error_overflow_too_many_digits();
std::string_view error_overflow_float_too_large();
std::string error_int_base(std::string_view fn_name);
std::string error_invalid_literal_with_base(std::string_view fn_name, int64_t base, std::string_view literal_value);

std::string error_no_keyword(std::string_view fn_name);
std::string error_unknown_argument(std::string_view argument_name);
std::string error_no_pos_args(std::string_view fn_name, int64_t actual);
std::string error_arguments_too_few(std::string_view fn_name, int64_t actual, int64_t expected);
std::string error_arguments_too_many(std::string_view fn_name, int64_t actual, int64_t expected);
std::string error_arguments_exactly_one(std::string_view fn_name, int64_t actual);
std::string error_arguments_one_or_two(std::string_view fn_name, int64_t actual);
std::string error_arguments_exactly(std::string_view fn_name, int64_t actual, int64_t expected);
std::string error_argument_bad_operand_type(std::string_view fn_name, std::string_view type);
std::string error_argument_bad_operand_type(std::string_view fn_name, std::string_view type, std::string_view expected1, std::string_view expected2);
std::string error_argument_string_or_real(std::string_view fn_name, std::string_view type);
std::string error_argument_string_int_bool_or_real(std::string_view fn_name, std::string_view type);
std::string error_attribute_string(std::string_view type);
std::string error_expect_character(std::string_view fn_name, std::string_view type, int64_t length);
std::string error_expect_one_character_or_one_byte(std::string_view fn_name, std::string_view type);
std::string error_argument_non_zero(std::string_view fn_name, int64_t arg_pos);
std::string error_step_non_zero();

std::string error_op_in_loop(std::string_view type, std::string_view op);
std::string error_mutate_frozen_value(std::string_view type);

std::string error_no_concat(std::string_view this_type, std::string_view other_type);
std::string error_no_concat(std::string_view this_type, std::string_view other_type, std::string_view possible_type);
std::string error_no_multiply_sequence(std::string_view other_type);

std::string error_max_sequence_length(int64_t max_length);
std::string error_unpack_too_few(int64_t expected, int64_t actual);
std::string error_unpack_too_many(int64_t expected, int64_t actual);
std::string error_in_element(std::string_view type, std::string_view actual, std::string_view expected);

std::string_view error_incomplete_format();
std::string_view error_not_enough_arguments_for_format_string();
std::string_view error_not_all_arguments_converted_during_string_formatting();
std::string error_unsupported_format_character(char c, std::size_t pos);
std::string error_format_integer_is_required(char format, std::string_view type);
std::string error_format_real_is_required(char format, std::string_view type);
std::string error_integer_or_unicode_character(std::string_view type);
std::string error_integer_or_unicode_character_type_and_length(std::string_view type, int64_t len);

std::string error_integer_or_type(std::string_view type, std::string_view other_type);
std::string error_argument_must_be_type(std::string_view fn_name, int arg_num, std::string_view expected_type, std::string_view actual_type);
std::string error_named_argument_must_be_type(std::string_view fn_name, std::string_view arg_name, std::string_view expected_type, std::string_view actual_type);
std::string error_tuple_must_contain_type(std::string_view fn_name, std::string_view expected_type, std::string_view actual_type);
std::string error_string_or_tuple_of_string(std::string_view fn_name, std::string_view actual_type);
std::string error_bytes_or_tuple_of_bytes(std::string_view fn_name, std::string_view actual_type);
std::string error_type_required(std::string_view expected_type, std::string_view actual_type);

std::string_view error_substring_not_found();
std::string error_non_iterable(std::string_view fn_name);

std::string_view error_empty_separator();
std::string_view error_can_only_join_on_iterable();
std::string error_in_type_requires_type(std::string_view base, std::string_view required, std::string_view actual);
std::string error_multiple_values_for_argument(std::string_view fn_name, std::string_view argument_name);
std::string error_unexpected_keyword_argument(std::string_view fn_name, std::string_view argument_name);
std::string error_missing_positional_argument(std::string_view fn_name, std::string_view argument_name);
std::string error_missing_keyword_only_argument(std::string_view fn_name, std::string_view argument_name);
std::string error_missing_argument(std::string_view fn_name, std::string_view argument_name);
std::string error_missing_typed_argument(std::string_view fn_name, std::string_view type_name);

std::string error_empty_iterator(std::string_view fn_name);

std::string error_unexpected_in_field_name(std::string_view unexpected);
std::string_view error_switch_from_manual_to_automatic_numbering();
std::string error_single_format_element_in_string(std::string_view format_element);
std::string error_expected_format_element_before_end_of_string(std::string_view format_element);
std::string error_positional_argument_out_of_range(std::string_view pos_argument);
std::string error_positional_argument_out_of_range(std::size_t pos_argument);
std::string_view error_end_of_string_while_looking_for_conversion_specifier();
std::string error_unknown_conversion(std::string_view conversion);
std::string_view error_expected_after_conversion();
std::string_view error_non_string_with_base();
std::string_view error_keyword_must_be_string();

std::string error_dictionary_duplicate_key(std::string_view key);
std::string error_unbound_variable(std::string_view name);
std::string error_symbol_not_available(std::string_view symbol);
std::string error_expect_mapping_after_star_star(std::string_view type);
std::string error_multiple_values_for_keyword(std::string_view name);
std::string error_unable_to_load_module(std::string_view module_name);
std::string error_module_not_ready(std::string_view module_name);
std::string error_module_does_not_define_symbol(std::string_view  module_name, std::string_view symbol);
std::string error_unknown_op(int op_code);
std::string error_recursive_call(std::string_view fn_name);

}  // namespace error_messages
}  // namespace starlark

#pragma GCC visibility pop

#endif  // ERRORS_RUNTIME_ERROR_MESSAGES_HPP_

