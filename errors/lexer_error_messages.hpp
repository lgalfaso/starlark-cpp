// Copyright 2026 Lucas Mirelmann

#ifndef ERRORS_LEXER_ERROR_MESSAGES_HPP_
#define ERRORS_LEXER_ERROR_MESSAGES_HPP_

#include <string>
#include <string_view>

#include "proto/starlark_logging.pb.h"

#pragma GCC visibility push(default)

namespace starlark {
namespace error_messages {

std::string error_v2_dangling_bracket(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_unexpected_character(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_wrong_indentation(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_unable_to_parse_numeric_literal(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_unterminated_string(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_invalid_line_continuation(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_invalid_escape_sequence_octal(int max_octal, std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_invalid_escape_sequence_hex(int max_octal, std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_invalid_escape_sequence_unicode(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_invalid_escape_sequence_unicode_long(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_invalid_escape_sequence_unknown(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);
std::string error_v2_invalid_escape_sequence_N(std::string_view program, const starlark::logging::Position& start, const starlark::logging::Position& end);

}  // namespace error_messages
}  // namespace starlark

#pragma GCC visibility pop

#endif  // ERRORS_LEXER_ERROR_MESSAGES_HPP_

