// Copyright 2025 Lucas Mirelmann

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
  // TODO(lmirelmann): Check whether we can downgrade.
  return Arena::Create<starlark_bigint>(&arena, std::move(value));
}

starlark_obj* create_integer(const number& value, Arena& arena) {
  // TODO(lmirelmann): Check whether we can downgrade.
  return Arena::Create<starlark_bigint>(&arena, value);
}

starlark_obj* create_float(double value, Arena& arena) {
  return Arena::Create<starlark_float>(&arena, value);
}

}  // namespace runtime
}  // namespace starlark

