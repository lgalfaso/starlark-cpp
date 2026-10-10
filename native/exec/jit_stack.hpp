// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_EXEC_JIT_STACK_HPP_
#define NATIVE_EXEC_JIT_STACK_HPP_

#include <cstdint>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {
struct frame;
}
namespace native {

struct jit_value_stack {
  starlark::runtime::starlark_obj** data = nullptr;
  uint32_t size = 0;
  uint32_t capacity = 0;

  void bind(starlark::runtime::starlark_obj** buffer, uint32_t cap);
  void clear();
  void push(starlark::runtime::starlark_obj* value);
  starlark::runtime::starlark_obj* pop();
  starlark::runtime::starlark_obj* peek() const;
  starlark::runtime::starlark_obj* at(uint32_t index) const;
  void resize(uint32_t new_size);
};

struct jit_frame_chain {
  starlark::vm::frame** data = nullptr;
  uint32_t depth = 0;
  uint32_t capacity = 0;

  void bind(starlark::vm::frame** buffer, uint32_t cap);
  void clear();
  void push(starlark::vm::frame* frame);
  starlark::vm::frame* back() const;
  starlark::vm::frame* at(uint32_t index_from_back) const;
  void resize(uint32_t new_depth);
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_EXEC_JIT_STACK_HPP_
