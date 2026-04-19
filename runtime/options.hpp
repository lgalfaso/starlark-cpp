// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_OPTIONS_HPP_
#define RUNTIME_OPTIONS_HPP_

#include <cstddef>
#include <cstdint>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

int64_t log2_max_bigint();
std::size_t max_sequence_size();
std::size_t max_string_length();

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_OPTIONS_HPP_

