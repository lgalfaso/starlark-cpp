// Copyright 2026 Lucas Mirelmann

#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_object.hpp"
#include "runtime/starlark_string.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

context::context(Arena& arena) : context(arena, runtime_options{}) {}

context::context(Arena& arena, const runtime_options& options) :
  arena_(arena),
  options_(options),
  false_value_(Arena::Create<starlark_bool>(&arena, false)),
  true_value_(Arena::Create<starlark_bool>(&arena, true)),
  none_value_(Arena::Create<starlark_none>(&arena)),
  minus_one_(Arena::Create<starlark_integer>(&arena, -1)),
  zero_(Arena::Create<starlark_integer>(&arena, 0)),
  one_(Arena::Create<starlark_integer>(&arena, 1)),
  empty_bytes_(Arena::Create<starlark_bytes>(&arena, std::string_view())),
  empty_string_(Arena::Create<starlark_string>(&arena, std::string_view())) {}

starlark_obj* context::false_value() const {
  return false_value_;
}

starlark_obj* context::true_value() const {
  return true_value_;
}

starlark_obj* context::none_value() const {
  return none_value_;
}

starlark_obj* context::minus_one() const {
  return minus_one_;
}

starlark_obj* context::zero() const {
  return zero_;
}

starlark_obj* context::one() const {
  return one_;
}

starlark_obj* context::empty_bytes() const {
  return empty_bytes_;
}

starlark_obj* context::empty_string() const {
  return empty_string_;
}

Arena& context::arena() {
  return arena_;
}

const runtime_options& context::options() {
  return options_;
}

}  // namespace runtime
}  // namespace starlark

