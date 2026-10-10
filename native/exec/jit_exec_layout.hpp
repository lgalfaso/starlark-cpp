// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_EXEC_JIT_EXEC_LAYOUT_HPP_
#define NATIVE_EXEC_JIT_EXEC_LAYOUT_HPP_

#include <cstddef>

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct jit_exec_layout {
  static std::size_t offsetof_stack_data();
  static std::size_t offsetof_stack_size();
  static std::size_t offsetof_frame_depth();
  static std::size_t offsetof_frame_chain_frames();
  static std::size_t offsetof_failed();
  static std::size_t offsetof_mod_error_block();
  static std::size_t offsetof_mod_error_ip();
  static std::size_t offsetof_frame_elements(void* frame);
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_EXEC_JIT_EXEC_LAYOUT_HPP_
