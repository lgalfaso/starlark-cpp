// Copyright 2025 Lucas Mirelmann

#ifndef GRAMMAR_PARSING_OPTIONS_HPP_
#define GRAMMAR_PARSING_OPTIONS_HPP_

#include <string_view>

#include "grammar/options.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

options get_parsing_options(std::string_view starlark_program);

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_PARSING_OPTIONS_HPP_

