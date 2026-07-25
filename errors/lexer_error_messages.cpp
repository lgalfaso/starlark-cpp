// Copyright 2026 Lucas Mirelmann

#include "errors/lexer_error_messages.hpp"

#include <format>
#include <string>
#include <string_view>

namespace starlark {
namespace error_messages {

std::string_view error_dangling_bracket() {
  return "Dangling bracket";
}

std::string_view error_unexpected_character() {
  return "Unexpected character";
}

std::string_view error_wrong_indentation() {
  return "Indentation error";
}

std::string_view error_unable_to_parse_numeric_literal() {
  return "Unable to parse numeric value";
}

std::string_view error_unterminated_string() {
  return "Unterminated string";
}

std::string_view error_invalid_line_continuation() {
  return "Invalid line continuation";
}

std::string_view error_invalid_escape_sequence() {
  return "Invalid escape sequence";
}

std::string_view error_invalid_escape_sequence_N() {
  return "Invalid escape sequence, the escape sequence \\N is not supported.";
}

}  // namespace error_messages
}  // namespace starlark


