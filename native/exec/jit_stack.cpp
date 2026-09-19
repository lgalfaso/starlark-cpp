// Copyright 2026 Lucas Mirelmann

#include "native/exec/jit_stack.hpp"

#include <cassert>

#include <vector>

namespace starlark {
namespace native {

void jit_value_stack::bind(starlark::runtime::starlark_obj** buffer, uint32_t cap) {
  data = buffer;
  capacity = cap;
  size = 0;
}

void jit_value_stack::clear() {
  size = 0;
}

void jit_value_stack::push(starlark::runtime::starlark_obj* value) {
  assert(data != nullptr);
  assert(size < capacity);
  data[size++] = value;
}

starlark::runtime::starlark_obj* jit_value_stack::pop() {
  assert(size > 0);
  return data[--size];
}

starlark::runtime::starlark_obj* jit_value_stack::peek() const {
  assert(size > 0);
  return data[size - 1];
}

starlark::runtime::starlark_obj* jit_value_stack::at(uint32_t index) const {
  assert(index < size);
  return data[index];
}

void jit_value_stack::resize(uint32_t new_size) {
  assert(new_size <= capacity);
  size = new_size;
}

void jit_frame_chain::bind(starlark::vm::frame** buffer, uint32_t cap) {
  data = buffer;
  capacity = cap;
  depth = 0;
}

void jit_frame_chain::clear() {
  depth = 0;
}

void jit_frame_chain::push(starlark::vm::frame* frame) {
  assert(data != nullptr);
  assert(depth < capacity);
  data[depth++] = frame;
}

starlark::vm::frame* jit_frame_chain::back() const {
  assert(depth > 0);
  return data[depth - 1];
}

starlark::vm::frame* jit_frame_chain::at(uint32_t index_from_back) const {
  assert(index_from_back < depth);
  return data[depth - 1 - index_from_back];
}

void jit_frame_chain::resize(uint32_t new_depth) {
  assert(new_depth <= depth);
  depth = new_depth;
}

}  // namespace native
}  // namespace starlark
