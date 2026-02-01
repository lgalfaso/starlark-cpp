// Copyright 2026 Lucas Mirelmann

#ifndef RUNTIME_ERROR_MESSAGES_HPP_
#define RUNTIME_ERROR_MESSAGES_HPP_

#include <string>
#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

std::string error_not_callable(std::string_view type);
std::string error_unpackable(std::string_view type);
std::string error_unhashable_key(std::string_view type, std::string_view key_type);
std::string error_unhashable_value(std::string_view type, std::string_view value_type);
std::string error_empty_dictionary(std::string_view fn_name);
std::string error_uniterable(std::string_view type);
std::string error_argument_uniterable(std::string_view type);
std::string error_unsubscriptable(std::string_view type);
std::string error_no_item_assignment(std::string_view type);
std::string error_incomparable(std::string_view op, std::string_view type1, std::string_view type2);
std::string error_index_out_of_range(std::string_view type);
std::string error_index_integer_or_slice(std::string_view type, std::string_view actual);

std::string error_bad_operand_unary(std::string_view op, std::string_view type);
std::string error_bad_operand_binary(std::string_view op, std::string_view type1, std::string_view type2);
std::string error_no_method(std::string_view type, std::string_view method);
std::string error_no_attribute(std::string_view type, std::string_view attribute);
std::string error_no_attribute(std::string_view type, std::string_view attribute, std::string_view suggestion);
std::string error_read_only_attribute(std::string_view type, std::string_view attribute);

std::string error_convert(std::string_view from, std::string_view to);
std::string error_convert_string(std::string_view to, std::string_view string_value);
std::string error_convert_non_string_with_base(std::string_view fn_name);
std::string_view error_convert_float_infinity_to_integer();
std::string_view error_convert_float_nan_to_integer();
std::string error_interpreted_as_integer(std::string_view type);
std::string error_argument_interpreted_as_integer(std::string_view argument_name, std::string_view type);

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

std::string error_op_in_loop(std::string_view type, std::string_view op);
std::string error_mutate_frozen_value(std::string_view type);

std::string error_like_required(std::string_view like_type, std::string_view actual_type);
std::string error_no_concat(std::string_view this_type, std::string_view other_type);
std::string error_no_concat(std::string_view this_type, std::string_view other_type, std::string_view possible_type);
std::string error_no_multiply_sequence(std::string_view other_type);

std::string error_max_sequence_length(int64_t max_length);
std::string error_unpack_too_few(int64_t expected, int64_t actual);
std::string error_unpack_too_many(int64_t expected, int64_t actual);
std::string error_in_element(std::string_view type, std::string_view actual, std::string_view expected);


}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_ERROR_MESSAGES_HPP_

