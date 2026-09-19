// Copyright 2026 Lucas Mirelmann

#include "native/exec/module_runtime_state.hpp"

#include "google/protobuf/arena.h"
#include "native/exec/native_exec_context.hpp"
#include "runtime/starlark_string.hpp"

using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_string;

namespace starlark {
namespace native {

namespace {

constexpr uint32_t kInitialFrameBufferCapacity = 32;

void bind_frame_chain(native_exec_context* exec) {
  if (exec == nullptr) {
    return;
  }
  if (exec->frame_buffer.empty()) {
    exec->frame_buffer.resize(kInitialFrameBufferCapacity);
  }
  exec->frame_chain.bind(exec->frame_buffer.data(), static_cast<uint32_t>(exec->frame_buffer.size()));
}

}  // namespace

module_runtime_state::module_runtime_state() {
  exec_context = std::make_unique<native_exec_context>();
}

module_runtime_state::~module_runtime_state() = default;

void module_runtime_state::materialize_const_strings() {
  const_strings.clear();
  if (program == nullptr || native_ctx == nullptr) {
    return;
  }
  const int count = program->const_string_size();
  if (count == 0) {
    return;
  }
  const_strings.resize(static_cast<std::size_t>(count));
  for (int i = 0; i < count; ++i) {
    const_strings[static_cast<std::size_t>(i)] =
        google::protobuf::Arena::Create<starlark_string>(&native_ctx->arena(), program->const_string(i));
  }
}

void module_runtime_state::prepare_rerun() {
  arena.Reset();
  const_strings.clear();
  function_frame_cache.clear();
  init_done = false;
  compatibility_frame = nullptr;
  native_ctx = nullptr;
  fns_in_stack.clear();
  active_exec_stack.clear();
  error_block_ptr = 0;
  error_ip = 0;
  if (exec_context != nullptr) {
    exec_context->eval_stack.clear();
    exec_context->frame_chain.clear();
    exec_context->module_result_frame = nullptr;
    exec_context->failed = false;
    exec_context->ctx = nullptr;
    exec_context->err = nullptr;
  }
}

void native_exec_context::reset(module_runtime_state* state,
    context* context,
    error_fn* error,
    std::span<starlark::vm::frame* const> closure_frames) {
  mod = state;
  code_mod = state;
  ctx = context;
  err = error;
  if (state != nullptr) {
    eval_stack.bind(state->stack_buffer.data(), static_cast<uint32_t>(state->stack_buffer.size()));
  } else {
    eval_stack.bind(nullptr, 0);
  }
  eval_stack.clear();
  bind_frame_chain(this);
  frame_chain.clear();
  for (auto* frame : closure_frames) {
    native_frame_chain_push(this, frame);
  }
  module_result_frame = nullptr;
  frame_pool_by_slot_count.clear();
  inner = state != nullptr && state->inner_module;
  failed = false;
}

}  // namespace native
}  // namespace starlark
