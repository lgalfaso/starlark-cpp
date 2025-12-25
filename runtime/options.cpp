// Copyright 2025 Lucas Mirelmann

#include "runtime/options.hpp"

#include <limits>

namespace starlark {
namespace runtime {

int64_t log2_max_bigint() {
  // TODO(lmirelmann): Make this configurable.
  return 30;  // 2**30.
}

int64_t max_sequence_size() {
  // TODO(lmirelmann): Make this configurable.
  // TODO(lmirelmann): This belongs to runtime options.
  return std::numeric_limits<int32_t>::max();
}

int64_t max_string_length() {
  // TODO(lmirelmann): Make this configurable.
  // TODO(lmirelmann): This belongs to runtime options.
  return std::numeric_limits<int32_t>::max();
}

}  // namespace runtime
}  // namespace starlark

