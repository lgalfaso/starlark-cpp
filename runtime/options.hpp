// Copyright 2025-2026 Lucas Mirelmann

#ifndef RUNTIME_OPTIONS_HPP_
#define RUNTIME_OPTIONS_HPP_

#include <cstddef>
#include <cstdint>

#include <iostream>
#include <limits>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

struct runtime_options {
  int64_t log2_max_bigint = 1'073'741'824;  // 2**30.
  std::size_t max_sequence_size = std::numeric_limits<int32_t>::max();
  std::size_t max_string_length = std::numeric_limits<int32_t>::max();
  std::ostream& out = std::cout;
  bool allow_recursion = false;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_OPTIONS_HPP_

