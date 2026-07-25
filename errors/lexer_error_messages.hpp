// Copyright 2026 Lucas Mirelmann

#ifndef ERRORS_LEXER_ERROR_MESSAGES_HPP_
#define ERRORS_LEXER_ERROR_MESSAGES_HPP_

#include <string>
#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace error_messages {

std::string_view error_dangling_bracket();
std::string_view error_unexpected_character();
std::string_view error_wrong_indentation();
std::string_view error_unable_to_parse_numeric_literal();
std::string_view error_unterminated_string();
std::string_view error_invalid_line_continuation();
std::string_view error_invalid_escape_sequence();
std::string_view error_invalid_escape_sequence_N();

}  // namespace error_messages
}  // namespace starlark

#pragma GCC visibility pop

#endif  // ERRORS_LEXER_ERROR_MESSAGES_HPP_

