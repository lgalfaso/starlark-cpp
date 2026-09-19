// Copyright 2026 Lucas Mirelmann

#include "native/exec/native_exec_context.hpp"

namespace starlark {
namespace native {

void native_frame_chain_push(native_exec_context* exec, starlark::vm::frame* frame) {
  if (exec == nullptr) {
    return;
  }
  if (exec->frame_buffer.empty()) {
    exec->frame_buffer.resize(32);
  }
  if (exec->frame_chain.capacity < exec->frame_chain.depth + 1) {
    uint32_t new_cap = exec->frame_chain.capacity * 2 + 64;
    if (new_cap < exec->frame_chain.depth + 1) {
      new_cap = exec->frame_chain.depth + 1;
    }
    exec->frame_buffer.resize(new_cap);
    exec->frame_chain.data = exec->frame_buffer.data();
    exec->frame_chain.capacity = static_cast<uint32_t>(exec->frame_buffer.size());
  }
  exec->frame_chain.push(frame);
}

}  // namespace native
}  // namespace starlark
