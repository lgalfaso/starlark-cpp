// Copyright 2026 Lucas Mirelmann

#include "errors/lexer_error_messages.hpp"

#include <format>
#include <string>
#include <string_view>

#include "errors/source_highlight.hpp"
#include "proto/starlark_logging.pb.h"

using ::starlark::logging::Position;

namespace starlark {
namespace error_messages {

std::string error_v2_dangling_bracket(std::string_view program, const Position& start, const Position& end) {
  return std::format("dangling bracket\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_unexpected_character(std::string_view program, const Position& start, const Position& end) {
  return std::format("unexpected character\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_wrong_indentation(std::string_view program, const Position& start, const Position& end) {
  return std::format("unindent does not match any outer indentation level\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_unable_to_parse_numeric_literal(std::string_view program, const Position& start, const Position& end) {
  return std::format("unable to parse numeric value\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_unterminated_string(std::string_view program, const Position& start, const Position& end) {
  return std::format("unterminated string\n{}", get_line_and_underline(program, start, previous_position(program, end), end, end, ""));
}

std::string error_v2_invalid_line_continuation(std::string_view program, const Position& start, const Position& end) {
  return std::format("invalid line continuation\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_invalid_escape_sequence_octal(int max_octal, std::string_view program, const Position& start, const Position& end) {
  return std::format("invalid octal escape sequence. Octal escape sequences must be in the range 0-{}\n{}", max_octal, get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_invalid_escape_sequence_hex(int max_hex, std::string_view program, const Position& start, const Position& end) {
  return std::format("invalid hexadecimal escape sequence. Hexadecimal escape sequences must be exactly 2 hexadecimal digits and in the range 0-{}\n{}", max_hex, get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_invalid_escape_sequence_unicode(std::string_view program, const Position& start, const Position& end) {
  return std::format("invalid Unicode escape sequence. The escape sequence must be exactly 4 digits and cannot contain surrogates\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_invalid_escape_sequence_unicode_long(std::string_view program, const Position& start, const Position& end) {
  return std::format("invalid Unicode escape sequence. The escape sequence must be exactly 8 digits, cannot contain surrogates and must be in the range 0-0x10FFFF\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_invalid_escape_sequence_N(std::string_view program, const Position& start, const Position& end) {
  return std::format("invalid escape sequence, the escape sequence \\N is not supported\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

std::string error_v2_invalid_escape_sequence_unknown(std::string_view program, const Position& start, const Position& end) {
  return std::format("invalid escape sequence. The escape sequence is unknown\n{}", get_line_and_underline(program, start, start, end, end, ""));
}

}  // namespace error_messages
}  // namespace starlark


