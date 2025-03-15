// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_HEX_ENCODER_HPP_
#define COMPILER_HEX_ENCODER_HPP_

#include <string>

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

void write_printable(uint64_t codepoint, bool use_single_quote, bool allow_non_ascii_printable, std::string& output);

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_HEX_ENCODER_HPP_

