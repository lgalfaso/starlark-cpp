// Copyright 2024-2025 Lucas Mirelmann

#ifndef GRAMMAR_QUOTED_HPP_
#define GRAMMAR_QUOTED_HPP_

#include <string>
#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace grammar {

std::string quoted(std::string_view input);

}  // namespace grammar
}  // namespace starlark

#pragma GCC visibility pop

#endif  // GRAMMAR_QUOTED_HPP_
