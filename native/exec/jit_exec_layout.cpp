// Copyright 2026 Lucas Mirelmann

#include "native/exec/jit_exec_layout.hpp"

#include <cstddef>

#include "native/exec/native_exec_context.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "vm/frame.hpp"

namespace starlark {
namespace native {

std::size_t jit_exec_layout::offsetof_stack_data() {
  return offsetof(native_exec_context, eval_stack) + offsetof(jit_value_stack, data);
}

std::size_t jit_exec_layout::offsetof_stack_size() {
  return offsetof(native_exec_context, eval_stack) + offsetof(jit_value_stack, size);
}

std::size_t jit_exec_layout::offsetof_frame_depth() {
  return offsetof(native_exec_context, frame_chain) + offsetof(jit_frame_chain, depth);
}

std::size_t jit_exec_layout::offsetof_frame_chain_frames() {
  return offsetof(native_exec_context, frame_chain) + offsetof(jit_frame_chain, data);
}

std::size_t jit_exec_layout::offsetof_failed() {
  return offsetof(native_exec_context, failed);
}

std::size_t jit_exec_layout::offsetof_mod_error_block() {
  return offsetof(module_runtime_state, error_block_ptr);
}

std::size_t jit_exec_layout::offsetof_mod_error_ip() {
  return offsetof(module_runtime_state, error_ip);
}

std::size_t jit_exec_layout::offsetof_frame_elements(void* frame) {
  (void)frame;
  return offsetof(starlark::vm::frame, elements);
}

}  // namespace native
}  // namespace starlark
