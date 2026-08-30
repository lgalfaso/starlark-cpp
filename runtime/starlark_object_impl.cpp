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
    empty_bytes_(Arena::Create<starlark_bytes>(&arena, std::string_view())),
    empty_string_(Arena::Create<starlark_string>(&arena, std::string_view())) {
  runner_context_ = nullptr;
  for (int i = MIN_SMALL_INT; i <= MAX_SMALL_INT; ++i) {
    auto* integer = Arena::Create<starlark_integer>(&arena, i);
    (void)integer->hash();
    small_integers[i - MIN_SMALL_INT] = integer;
  }
}

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
  return small_integers[-1 - MIN_SMALL_INT];
}

starlark_obj* context::zero() const {
  return small_integers[0 - MIN_SMALL_INT];
}

starlark_obj* context::one() const {
  return small_integers[1 - MIN_SMALL_INT];
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

void* context::runner_context() {
  return runner_context_;
}

void context::runner_context(void* r_context) {
  runner_context_ = r_context;
}

}  // namespace runtime
}  // namespace starlark

