// Copyright 2026 Lucas Mirelmann

#include "gtest/gtest.h"
#include "native/exec/jit_exec_layout.hpp"
#include "native/exec/native_exec_context.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "vm/frame.hpp"

using ::starlark::native::jit_exec_layout;
using ::starlark::native::jit_frame_chain;
using ::starlark::native::jit_value_stack;
using ::starlark::native::module_runtime_state;
using ::starlark::native::native_exec_context;

TEST(JitExecLayout, OffsetsMatchNativeExecContext) {
  EXPECT_EQ(jit_exec_layout::offsetof_stack_data(), offsetof(native_exec_context, eval_stack) + offsetof(jit_value_stack, data));
  EXPECT_EQ(jit_exec_layout::offsetof_stack_size(), offsetof(native_exec_context, eval_stack) + offsetof(jit_value_stack, size));
  EXPECT_EQ(jit_exec_layout::offsetof_frame_depth(), offsetof(native_exec_context, frame_chain) + offsetof(jit_frame_chain, depth));
  EXPECT_EQ(jit_exec_layout::offsetof_frame_chain_frames(), offsetof(native_exec_context, frame_chain) + offsetof(jit_frame_chain, data));
  EXPECT_EQ(jit_exec_layout::offsetof_failed(), offsetof(native_exec_context, failed));
  EXPECT_EQ(jit_exec_layout::offsetof_mod_error_block(), offsetof(module_runtime_state, error_block_ptr));
  EXPECT_EQ(jit_exec_layout::offsetof_mod_error_ip(), offsetof(module_runtime_state, error_ip));
  EXPECT_EQ(jit_exec_layout::offsetof_frame_elements(nullptr), offsetof(starlark::vm::frame, elements));
}
