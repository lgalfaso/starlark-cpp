// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_OPTIONS_HPP_
#define RUNTIME_OPTIONS_HPP_

#include <cstdint>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

int64_t log2_max_bigint();
int64_t max_sequence_size();
int64_t max_string_length();

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_OPTIONS_HPP_

