// Copyright 2026 Lucas Mirelmann

#include "runtime/error_messages.hpp"

#include <format>
#include <string>
#include <string_view>

namespace starlark {
namespace runtime {

std::string error_not_callable(std::string_view type) {
  return std::format("TypeError: '{}' object is not callable", type);
}

std::string error_unpackable(std::string_view type) {
  return std::format("TypeError: cannot unpack non-iterable {} object", type);
}

std::string error_unhashable_key(std::string_view type, std::string_view key_type) {
  return std::format("TypeError: cannot use '{}' as a {} key (unhashable type: '{}')", key_type, type, key_type);
}

std::string error_unhashable_value(std::string_view type, std::string_view value_type) {
  return std::format("TypeError: cannot use '{}' as a {} element (unhashable type: '{}')", value_type, type, value_type);
}

std::string error_empty_dictionary(std::string_view fn_name) {
  return std::format("KeyError: '{}(): dictionary is empty'", fn_name);
}

std::string error_uniterable(std::string_view type) {
  return std::format("TypeError: '{}' object is not iterable", type);
}

std::string error_argument_uniterable(std::string_view type) {
  return std::format("TypeError: argument of type '{}' is not a container or iterable", type);
}

std::string error_unsubscriptable(std::string_view type) {
  return std::format("TypeError: '{}' object is not subscriptable", type);
}

std::string error_no_item_assignment(std::string_view type) {
  return std::format("TypeError: '{}' object does not support item assignment", type);
}

std::string error_incomparable(std::string_view op, std::string_view type1, std::string_view type2) {
  return std::format("TypeError: '{}' not supported between instances of '{}' and '{}'", op, type1, type2);
}

std::string error_index_out_of_range(std::string_view type) {
  return std::format("IndexError: {} index out of range", type);
}

std::string error_index_integer_or_slice(std::string_view type, std::string_view actual) {
  return std::format("TypeError: {} indices must be integers or slices, not '{}'", type, actual);
}

std::string error_index_integer_on_a_slice(std::string_view actual) {
  return std::format("TypeError: slice indices must be integers, not '{}'", actual);
}

std::string error_item_not_in_collection(std::string_view type, std::string_view fn_name) {
  return std::format("ValueError: {}.{}(x): x not in {}", type, fn_name, type);
}

std::string error_bad_operand_unary(std::string_view op, std::string_view type) {
  return std::format("TypeError: bad operand type for unary {}: '{}'", op, type);
}

std::string error_bad_operand_binary(std::string_view op, std::string_view type1, std::string_view type2) {
  return std::format("TypeError: unsupported operand type(s) for {}: '{}' and '{}'", op, type1, type2);
}

std::string error_no_method(std::string_view type, std::string_view method) {
  return std::format("TypeError: object of type '{}' has no {}()", type, method);
}

std::string error_no_attribute(std::string_view type, std::string_view attribute) {
  return std::format("AttributeError: '{}' object has no attribute '{}'", type, attribute);
}

std::string error_no_attribute(std::string_view type, std::string_view attribute, std::string_view suggestion) {
  return std::format("AttributeError: '{}' object has no attribute '{}'. Did you mean: '{}'?", type, attribute, suggestion);
}

std::string error_read_only_attribute(std::string_view type, std::string_view attribute) {
  return std::format("AttributeError: '{}' object attribute '{}' is read-only", type, attribute);
}

std::string error_convert(std::string_view from, std::string_view to) {
  return std::format("TypeError: cannot convert '{}' object to {}", from, to);
}

std::string error_convert_string(std::string_view to, std::string_view string_value) {
  return std::format("ValueError: could not convert string to {}: '{}'", to, string_value);
}

std::string error_convert_non_string_with_base(std::string_view fn_name) {
  return std::format("TypeError: {}() can't convert non-string with explicit base", fn_name);
}

std::string_view error_convert_float_infinity_to_integer() {
  return "OverflowError: cannot convert float infinity to integer";
}

std::string_view error_convert_float_nan_to_integer() {
  return "ValueError: cannot convert float NaN to integer";
}

std::string error_interpreted_as_integer(std::string_view type) {
  return std::format("TypeError: '{}' object cannot be interpreted as an integer", type);
}

std::string error_argument_interpreted_as_integer(std::string_view argument_name, std::string_view type) {
  return std::format("TypeError: parameter '{}' cannot be interpreted as an integer ({}).", argument_name, type);
}

std::string error_dictionary_key_not_found(std::string_view key) {
  return std::format("KeyError: {}", key);
}

std::string error_dictionary_update_sequence(int64_t position, int64_t actual, int64_t expected) {
  return std::format("ValueError: dictionary update sequence element #{} has length {}; {} is required", position, actual, expected);
}

std::string_view error_byte_in_range() {
  return "ValueError: byte must be in range(0, 256)";
}

std::string_view error_bytes_in_range() {
  return "ValueError: bytes must be in range(0, 256)";
}

std::string_view error_unicode_in_range() {
  return "ValueError: Unicode code point must be in range(0, 0x110000)";
}

std::string_view error_negative_shift() {
  return "ValueError: negative shift count";
}

std::string_view error_division_by_zero() {
  return "ZeroDivisionError: division by zero";
}

std::string error_overflow(std::string_view from, std::string_view to) {
  return std::format("OverflowError: {} too large to convert to {}", from, to);
}

std::string_view error_overflow_too_many_digits() {
  return "OverflowError: too many digits in integer";
}

std::string_view error_overflow_float_too_large() {
  return "OverflowError: floating-point number too large";
}

std::string error_int_base(std::string_view fn_name) {
  return std::format("ValueError: {}() base must be >= 2 and <= 36, or 0", fn_name);
}

std::string error_invalid_literal_with_base(std::string_view fn_name, int64_t base, std::string_view literal_value) {
  return std::format("ValueError: invalid literal for {}() with base {}: '{}'", fn_name, base, literal_value);
}

std::string error_no_keyword(std::string_view fn_name) {
  return std::format("TypeError: {}() takes no keyword arguments", fn_name);
}

std::string error_unknown_argument(std::string_view argument_name) {
  return std::format("Unknown named argument '{}'.", argument_name);
}

std::string error_no_pos_args(std::string_view fn_name, int64_t actual) {
  return std::format("TypeError: {}() takes no arguments ({} given)", fn_name, actual);
}

std::string error_arguments_too_few(std::string_view fn_name, int64_t actual, int64_t expected) {
  return std::format("TypeError: {} expected at least {} argument, got {}", fn_name, expected, actual);
}

std::string error_arguments_too_many(std::string_view fn_name, int64_t actual, int64_t expected) {
  return std::format("TypeError: {} expected at most {} argument, got {}", fn_name, expected, actual);
}

std::string error_arguments_exactly_one(std::string_view fn_name, int64_t actual) {
  return std::format("TypeError: {}() takes exactly one argument ({} given)", fn_name, actual);
}

std::string error_arguments_one_or_two(std::string_view fn_name, int64_t actual) {
  return std::format("TypeError: {}() takes one or two argument ({} given)", fn_name, actual);
}

std::string error_arguments_exactly(std::string_view fn_name, int64_t actual, int64_t expected) {
  return std::format("TypeError: {} expected {} arguments, got {}", fn_name, expected, actual);
}

std::string error_argument_bad_operand_type(std::string_view fn_name, std::string_view type) {
  return std::format("TypeError: bad operand type for {}(): '{}'", fn_name, type);
}

std::string error_argument_bad_operand_type(std::string_view fn_name, std::string_view type, std::string_view expected1, std::string_view expected2) {
  return std::format("TypeError: in call to {}(), got value of type '{}', want '{}' or '{}'", fn_name, type, expected1, expected2);
}

std::string error_argument_string_or_real(std::string_view fn_name, std::string_view type) {
  return std::format("TypeError: {}() argument must be a string or a real number, not '{}'", fn_name, type);
}

std::string error_argument_string_int_bool_or_real(std::string_view fn_name, std::string_view type) {
  return std::format("TypeError: {}() argument must be a string, int, bool or a real number, not '{}'", fn_name, type);
}

std::string error_attribute_string(std::string_view type) {
  return std::format("TypeError: attribute name must be string, not '{}'", type);
}

std::string error_expect_character(std::string_view fn_name, std::string_view type, int64_t length) {
  return std::format("TypeError: {}() expected a character, but {} of length {} found", fn_name, type, length);
}

std::string error_expect_one_character_or_one_byte(std::string_view fn_name, std::string_view type) {
  return std::format("TypeError: {}() expected bytes of length 1 or string with one character, but '{}' found", fn_name, type);
}

std::string error_argument_non_zero(std::string_view fn_name, int64_t arg_pos) {
  return std::format("ValueError: {}() arg {} must not be zero", fn_name, arg_pos);
}

std::string error_step_non_zero() {
  return "ValueError: slice step cannot be zero";
}

std::string error_op_in_loop(std::string_view type, std::string_view op) {
  return std::format("Error in {}: {} value is temporarily immutable due to active for-loop iteration", op, type);
}

std::string error_mutate_frozen_value(std::string_view type) {
  // This error does not exists in Python, so using a mix of the Python error type and Bazel message.
  return std::format("TypeError: trying to mutate a frozen {} value", type);
}

std::string error_like_required(std::string_view like_type, std::string_view actual_type) {
  return std::format("TypeError: a {}-like object is required, not '{}'", like_type, actual_type);
}

std::string error_no_concat(std::string_view this_type, std::string_view other_type) {
  return std::format("TypeError: can't concat {} to {}", other_type, this_type);
}

std::string error_no_concat(std::string_view this_type, std::string_view other_type, std::string_view possible_type) {
  return std::format("TypeError: can only concatenate {} (not \"{}\") to {}", possible_type, other_type, this_type);
}

std::string error_no_multiply_sequence(std::string_view other_type) {
  return std::format("TypeError: can't multiply sequence by non-int of type '{}'", other_type);
}

std::string error_max_sequence_length(int64_t max_length) {
  return std::format("TypeError: sequences must be at most {} elements", max_length);
}

std::string error_unpack_too_few(int64_t actual, int64_t expected) {
  return std::format("ValueError: not enough values to unpack (expected {}, got {})", expected, actual);
}

std::string error_unpack_too_many(int64_t actual, int64_t expected) {
  return std::format("ValueError: too many values to unpack (expected {}, got {})", expected, actual);
}

std::string error_in_element(std::string_view type, std::string_view actual, std::string_view expected) {
  return std::format("TypeError: 'in <{}>' requires {} as left operand, not {}", type, expected, actual);
}

}  // namespace runtime
}  // namespace starlark

