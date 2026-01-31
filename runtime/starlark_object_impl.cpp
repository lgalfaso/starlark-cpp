// Copyright 2026 Lucas Mirelmann

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_object.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

context::context(Arena& arena) :
  arena_(arena),
  false_value_(Arena::Create<starlark_bool>(&arena, false)),
  true_value_(Arena::Create<starlark_bool>(&arena, true)),
  none_value_(Arena::Create<starlark_none>(&arena)),
  zero_(create_integer(0, arena)),
  one_(create_integer(1, arena)) {}

starlark_obj* context::false_value() const {
  return false_value_;
}

starlark_obj* context::true_value() const {
  return true_value_;
}

starlark_obj* context::none_value() const {
  return none_value_;
}

starlark_obj* context::zero() const {
  return zero_;
}

starlark_obj* context::one() const {
  return one_;
}

Arena& context::arena() {
  return arena_;
}

}  // namespace runtime
}  // namespace starlark

