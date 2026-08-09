// Copyright 2026 Lucas Mirelmann

#include "errors/runtime_error_messages.hpp"

#include <format>
#include <string>
#include <string_view>

#include "errors/source_highlight.hpp"
#include "proto/starlark_logging.pb.h"

using ::starlark::logging::Position;

namespace starlark {
namespace error_messages {

std::string error_convert(std::string_view from, std::string_view to) {
  return std::format("TypeError: cannot convert '{}' object to {}", from, to);
}

std::string error_convert_string(std::string_view to, std::string_view string_value) {
  return std::format("ValueError: could not convert string to {}: '{}'", to, string_value);
}

std::string_view error_convert_float_infinity_to_integer() {
  return "OverflowError: cannot convert float infinity to integer";
}

std::string_view error_convert_float_nan_to_integer() {
  return "ValueError: cannot convert float NaN to integer";
}

std::string error_argument_interpreted_as_integer(std::string_view argument_name, std::string_view type) {
  return std::format("TypeError: parameter '{}' cannot be interpreted as an integer ({})", argument_name, type);
}

std::string error_argument_interpreted_as_string(std::string_view argument_name, std::string_view type) {
  return std::format("TypeError: parameter '{}' cannot be interpreted as an string ({})", argument_name, type);
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
  return std::format("Unknown named argument '{}'", argument_name);
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

std::string error_max_string_length(int64_t max_length) {
  return std::format("TypeError: string must be at most {} elements", max_length);
}

std::string error_max_bytes_length(int64_t max_length) {
  return std::format("TypeError: bytes must be at most {} elements", max_length);
}

std::string error_in_element(std::string_view type, std::string_view actual, std::string_view expected) {
  return std::format("TypeError: 'in <{}>' requires {} as left operand, not {}", type, expected, actual);
}

std::string_view error_incomplete_format() {
  return "ValueError: incomplete format";
}

std::string_view error_not_enough_arguments_for_format_string() {
  return "TypeError: not enough arguments for format string";
}

std::string_view error_not_all_arguments_converted_during_string_formatting() {
  return "TypeError: not all arguments converted during string formatting";
}

std::string error_unsupported_format_character(char format, std::size_t pos) {
  return std::format("ValueError: unsupported format character '{}' (0x{:x}) at index {}", format, static_cast<int>(static_cast<unsigned char>(format)), pos);
}

std::string error_format_integer_is_required(char format, std::string_view type) {
  return std::format("TypeError: %{} format: an integer is required, not {}", format, type);
}

std::string error_format_real_is_required(char format, std::string_view type) {
  return std::format("TypeError: %{} format: a real number is required, not {}", format, type);
}

std::string error_integer_or_unicode_character(std::string_view type) {
  return std::format("TypeError: %c requires an int or a unicode character, not {}", type);
}

std::string error_integer_or_unicode_character_type_and_length(std::string_view type, int64_t len) {
  return std::format("TypeError: %c requires an int or a unicode character, not {} of length {}", type, len);
}

std::string error_integer_or_type(std::string_view type, std::string_view other_type) {
  return std::format("TypeError: argument should be integer or {} object, not '{}'", type, other_type);
}

std::string error_argument_must_be_type(std::string_view fn_name, int arg_num, std::string_view expected_type, std::string_view actual_type) {
  return std::format("TypeError: {}() argument {} must be {}, not {}", fn_name, arg_num, expected_type, actual_type);
}

std::string error_named_argument_must_be_type(std::string_view fn_name, std::string_view arg_name, std::string_view expected_type, std::string_view actual_type) {
  return std::format("TypeError: {}() argument {} must be {}, not {}", fn_name, arg_name, expected_type, actual_type);
}

std::string error_tuple_must_contain_type(std::string_view fn_name, std::string_view expected_type, std::string_view actual_type) {
  return std::format("TypeError: tuple for {} must only contain {}, not {}", fn_name, expected_type, actual_type);
}

std::string error_string_or_tuple_of_string(std::string_view fn_name, std::string_view actual_type) {
  return std::format("TypeError: {} first arg must be str or a tuple of str, not {}", fn_name, actual_type);
}

std::string error_bytes_or_tuple_of_bytes(std::string_view fn_name, std::string_view actual_type) {
  return std::format("TypeError: {} first arg must be bytes or a tuple of bytes, not {}", fn_name, actual_type);
}

std::string error_type_required(std::string_view expected_type, std::string_view actual_type) {
  return std::format("TypeError: a {} object is required, not '{}'", expected_type, actual_type);
}

std::string_view error_substring_not_found() {
  return "ValueError: substring not found";
}

std::string error_non_iterable(std::string_view fn_name) {
  return std::format("TypeError: can only {} an iterable", fn_name);
}

std::string_view error_empty_separator() {
  return "ValueError: empty separator";
}

std::string_view error_can_only_join_on_iterable() {
  return "TypeError: can only join an iterable";
}

std::string error_in_type_requires_type(std::string_view base, std::string_view required, std::string_view actual) {
  return std::format("TypeError: 'in <{}>' requires {} as left operand, not {}", base, required, actual);
}

std::string error_multiple_values_for_argument(std::string_view fn_name, std::string_view argument_name) {
  return std::format("TypeError: {}() got multiple values for argument '{}'", fn_name, argument_name);
}

std::string error_unexpected_keyword_argument(std::string_view fn_name, std::string_view argument_name) {
  return std::format("TypeError: {}() got an unexpected keyword argument '{}'", fn_name, argument_name);
}

std::string error_missing_positional_argument(std::string_view fn_name, std::string_view argument_name) {
  return std::format("TypeError: {}() missing required positional argument '{}'", fn_name, argument_name);
}

std::string error_missing_keyword_only_argument(std::string_view fn_name, std::string_view argument_name) {
  return std::format("TypeError: {}() missing required keyword-only argument '{}'", fn_name, argument_name);
}

std::string error_missing_argument(std::string_view fn_name, std::string_view argument_name) {
  return std::format("TypeError: {}() missing required argument '{}'", fn_name, argument_name);
}

std::string error_missing_typed_argument(std::string_view fn_name, std::string_view type_name) {
  return std::format("TypeError: {}() missing {} argument", fn_name, type_name);
}

std::string error_empty_iterator(std::string_view fn_name) {
  return std::format("ValueError: {}() iterable argument is empty", fn_name);
}

std::string error_unexpected_in_field_name(std::string_view unexpected) {
  return std::format("ValueError: unexpected '{}' in field name", unexpected);
}

std::string_view error_switch_from_manual_to_automatic_numbering() {
  return "ValueError: cannot switch from manual field specification to automatic field numbering";
}

std::string error_single_format_element_in_string(std::string_view format_element) {
  return std::format("ValueError: Single '{}' encountered in format string", format_element);
}

std::string error_expected_format_element_before_end_of_string(std::string_view format_element) {
  return std::format("ValueError: expected '{}' before end of string", format_element);
}

std::string error_positional_argument_out_of_range(std::string_view pos_argument) {
  return std::format("IndexError: Replacement index {} out of range for positional args tuple", pos_argument);
}

std::string error_positional_argument_out_of_range(std::size_t pos_argument) {
  return std::format("IndexError: Replacement index {} out of range for positional args tuple", pos_argument);
}

std::string_view error_end_of_string_while_looking_for_conversion_specifier() {
  return "ValueError: end of string while looking for conversion specifier";
}

std::string error_unknown_conversion(std::string_view conversion) {
  return std::format("ValueError: Unknown conversion specifier {}", conversion);
}

std::string_view error_expected_after_conversion() {
  // This is not the same error as in Python, but given that we do not support all
  // the formats from Python, this is the best we can do.
  return "ValueError: expected '}' after conversion specifier";
}

std::string_view error_non_string_with_base() {
  return "TypeError: int() can't convert non-string with explicit base";
}

std::string_view error_keyword_must_be_string() {
  return "TypeError: keywords must be strings";
}

std::string error_dictionary_duplicate_key(std::string_view key) {
  return std::format("Error: dictionary expression has duplicate key: {}", key);
}

std::string error_unbound_variable(std::string_view name) {
  return std::format("UnboundLocalError: cannot access local variable '{}' where it is not associated with a value", name);
}

std::string error_symbol_not_available(std::string_view symbol) {
  return std::format("Error: Required symbol {} not avaible in the global context", symbol);
}

std::string error_expect_mapping_after_star_star(std::string_view type) {
  return std::format("TypeError: argument after ** must be a mapping, not {}", type);
}

std::string error_multiple_values_for_keyword(std::string_view name) {
  return std::format("TypeError: got multiple values for keyword argument '{}'", name);
}

std::string error_unable_to_load_module(std::string_view module_name) {
  return std::format("ModuleNotFoundError: Unable to load module named '{}'", module_name);
}

std::string error_module_not_ready(std::string_view module_name) {
  return std::format("LoadError: Module '{}' is not ready to be used", module_name);
}

std::string error_module_does_not_define_symbol(std::string_view  module_name, std::string_view symbol) {
  return std::format("LoadError: Module '{}' does not contain the symbol {}", module_name, symbol);
}

std::string error_unknown_op(int op_code) {
  return std::format("Unknown op-code: {}", op_code);
}

std::string error_recursive_call(std::string_view fn_name) {
  return std::format("Error: function '{}' called recursively", fn_name);
}




std::string error_v2_max_string_length(int64_t max_length, std::string_view program, const Position& start, const Position& end) {
  return std::format("string must be at most {} elements\n{}", max_length, get_line_and_underline(program, start, end));
}

std::string error_v2_max_bytes_length(int64_t max_length, std::string_view program, const Position& start, const Position& end) {
  return std::format("bytes must be at most {} elements\n{}", max_length, get_line_and_underline(program, start, end));
}

std::string error_v2_max_sequence_length(int64_t max_length, std::string_view program, const Position& start, const Position& end) {
  return std::format("sequences must be at most {} elements\n{}", max_length, get_line_and_underline(program, start, end));
}

std::string error_v2_not_callable(std::string_view type) {
  return std::format("'{}' object is not callable", type);
}


std::string error_v2_unpackable(std::string_view type) {
  return std::format("cannot unpack non-iterable {} object", type);
}

std::string error_v2_unpack_too_few(int64_t actual, int64_t expected) {
  return std::format("not enough values to unpack (expected {}, got {})", expected, actual);
}

std::string error_v2_unpack_too_many(int64_t actual, int64_t expected) {
  return std::format("too many values to unpack (expected {}, got {})", expected, actual);
}


std::string error_v2_unhashable_key(std::string_view type, std::string_view key_type) {
  return std::format("cannot use '{}' as a {} key (unhashable type: '{}')", key_type, type, key_type);
}

std::string error_v2_unhashable_value(std::string_view type, std::string_view value_type) {
  return std::format("cannot use '{}' as a {} element (unhashable type: '{}')", value_type, type, value_type);
}


std::string error_v2_empty_dictionary(std::string_view fn_name) {
  return std::format("{}(): dictionary is empty", fn_name);
}

std::string error_v2_empty_set(std::string_view fn_name) {
  return std::format("{} from an empty set", fn_name);
}

std::string error_v2_uniterable(std::string_view type) {
  return std::format("'{}' object is not iterable", type);
}

std::string error_v2_argument_uniterable(std::string_view type) {
  return std::format("argument of type '{}' is not a container or iterable", type);
}

std::string error_v2_unsubscriptable(std::string_view type) {
  return std::format("'{}' object is not subscriptable", type);
}

std::string error_v2_no_item_assignment(std::string_view type) {
  return std::format("'{}' object does not support item assignment", type);
}

std::string error_v2_no_slice_assignment(std::string_view type) {
  return std::format("'{}' object does not support slice assignment", type);
}

std::string error_v2_incomparable(std::string_view op, std::string_view type1, std::string_view type2) {
  return std::format("'{}' not supported between instances of '{}' and '{}'", op, type1, type2);
}

std::string error_v2_index_out_of_range(std::string_view type) {
  return std::format("{} index out of range", type);
}

std::string error_v2_index_integer_or_slice(std::string_view type, std::string_view actual) {
  return std::format("{} indices must be integers or slices, not '{}'", type, actual);
}

std::string error_v2_index_integer_on_a_slice(std::string_view actual) {
  return std::format("slice indices must be integers, not '{}'", actual);
}

std::string error_v2_item_not_in_collection(std::string_view type, std::string_view fn_name) {
  return std::format("{}.{}(x): x not in {}", type, fn_name, type);
}

std::string error_v2_interpreted_as_integer(std::string_view type) {
  return std::format("'{}' object cannot be interpreted as an integer", type);
}


std::string error_v2_bad_operand_unary(std::string_view op, std::string_view type) {
  return std::format("bad operand type for unary {}: '{}'", op, type);
}

std::string error_v2_bad_operand_binary(std::string_view op, std::string_view type1, std::string_view type2) {
  return std::format("unsupported operand type(s) for {}: '{}' and '{}'", op, type1, type2);
}

std::string error_v2_no_method(std::string_view type, std::string_view method) {
  return std::format("object of type '{}' has no {}()", type, method);
}

std::string error_v2_no_attribute(std::string_view type, std::string_view attribute) {
  return std::format("'{}' object has no attribute '{}'", type, attribute);
}

std::string error_v2_no_attribute(std::string_view type, std::string_view attribute, std::string_view suggestion) {
  return std::format("'{}' object has no attribute '{}'. Did you mean: '{}'?", type, attribute, suggestion);
}

std::string error_v2_read_only_attribute(std::string_view type, std::string_view attribute) {
  return std::format("'{}' object attribute '{}' is read-only", type, attribute);
}

}  // namespace error_messages
}  // namespace starlark

