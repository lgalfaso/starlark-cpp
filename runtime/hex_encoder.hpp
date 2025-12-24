// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_HEX_ENCODER_HPP_
#define RUNTIME_HEX_ENCODER_HPP_

#include <string>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

void write_printable(uint64_t codepoint, bool allow_non_ascii_printable, std::string& output);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_HEX_ENCODER_HPP_

