// Copyright 2025 Lucas Mirelmann

#include <limits>
#include <utility>

#include "google/protobuf/arena.h"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_numeric.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_obj* create_integer(std::int64_t value, Arena& arena) {
  // TODO(lmirelmann): Use a cache of small integers.
  return Arena::Create<starlark_integer>(&arena, value);
}

starlark_obj* create_integer(number&& value, Arena& arena) {
  // Check whether we can downgrade.
  if (value.length() <= 1) {
    auto v = value.at(0);
    if (value.sign()) {
      if (v <= static_cast<uint64_t>(std::numeric_limits<int64_t>::min())) {
        return create_integer(-v, arena);
      }
    } else if (v <= std::numeric_limits<int64_t>::max()) {
      return create_integer(v, arena);
    }
  }
  return Arena::Create<starlark_bigint>(&arena, std::move(value));
}

starlark_obj* create_float(double value, Arena& arena) {
  return Arena::Create<starlark_float>(&arena, value);
}

}  // namespace runtime
}  // namespace starlark

