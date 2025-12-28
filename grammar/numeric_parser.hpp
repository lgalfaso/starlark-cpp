// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_NUMERIC_PARSER_HPP_
#define GRAMMAR_NUMERIC_PARSER_HPP_

#include <string>
#include <optional>

#include "unicode/utf8_reader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

std::optional<std::string> read_number(unicode::utf8_reader& input, bool allow_binary_literals);

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_NUMERIC_PARSER_HPP_

