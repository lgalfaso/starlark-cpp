// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_NATIVE_EXEC_CONTEXT_HPP_
#define NATIVE_NATIVE_EXEC_CONTEXT_HPP_

#include <cstdint>

#include <span>
#include <vector>

#include "native/exec/jit_stack.hpp"
#include "runtime/starlark_object.hpp"
#include "vm/frame.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct module_runtime_state;

struct native_exec_context {
  module_runtime_state* mod = nullptr;
  // Module whose bytecode is currently executing (for const-string pool lookup).
  module_runtime_state* code_mod = nullptr;

  jit_value_stack eval_stack;
  jit_frame_chain frame_chain;
  std::vector<starlark::vm::frame*> frame_buffer;
  std::vector<std::vector<starlark::vm::frame*>> frame_pool_by_slot_count;
  starlark::vm::frame* module_result_frame = nullptr;

  bool inner = false;
  bool failed = false;

  starlark::runtime::context* ctx = nullptr;
  starlark::runtime::error_fn* err = nullptr;
  void* runner_context = nullptr;

  void reset(module_runtime_state* state,
      starlark::runtime::context* context,
      starlark::runtime::error_fn* error,
      std::span<starlark::vm::frame* const> closure_frames = {});

  void reset_for_module_init(module_runtime_state* state, starlark::runtime::context* context, starlark::runtime::error_fn* error) {
    reset(state, context, error);
  }

  void reset_for_function_call(module_runtime_state* state,
      starlark::runtime::context* context,
      starlark::runtime::error_fn* error,
      const std::vector<starlark::vm::frame*>& closure_frames) {
    reset(state, context, error, closure_frames);
  }
};

inline starlark::runtime::context* effective_ctx(native_exec_context* exec, starlark::runtime::context* ctx) {
  return ctx != nullptr ? ctx : exec->ctx;
}

inline starlark::runtime::error_fn* effective_err(native_exec_context* exec, starlark::runtime::error_fn* err) {
  return err != nullptr ? err : exec->err;
}

void native_frame_chain_push(native_exec_context* exec, starlark::vm::frame* frame);

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_NATIVE_EXEC_CONTEXT_HPP_
