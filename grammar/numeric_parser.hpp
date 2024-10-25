// Copyright 2024 Lucas Mirelmann

#ifndef GRAMMAR_NUMERIC_PARSER_HPP_
#define GRAMMAR_NUMERIC_PARSER_HPP_

#include <string>
#include <optional>

#include "grammar/source.hpp"

#pragma GCC visibility push(default)

namespace grammar {

std::optional<std::string> read_number(source& input);

}  // namespace grammar

#pragma GCC visibility pop

#endif  // GRAMMAR_NUMERIC_PARSER_HPP_

