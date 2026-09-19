// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_MODULE_RUNTIME_STATE_HPP_
#define NATIVE_MODULE_RUNTIME_STATE_HPP_

#include <cstdint>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "google/protobuf/arena.h"
#include "google/protobuf/repeated_field.h"
#include "proto/starlark_bytecode.pb.h"
#include "runtime/options.hpp"
#include "runtime/starlark_object.hpp"
#include "vm/frame.hpp"
#include "vm/module_loader.hpp"
#include "vm/module_metadata.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct native_exec_context;

struct native_function_frame_cache {
  const google::protobuf::RepeatedPtrField<std::string>* names = nullptr;
  std::vector<int32_t> positional_slots;
  bool sequential_slots = false;
};

struct native_fn_stack_key {
  const starlark::bytecode::Program* program = nullptr;
  int32_t block_idx = 0;

  auto operator<=>(const native_fn_stack_key&) const = default;
};

struct module_runtime_state {
  module_runtime_state();
  ~module_runtime_state();

  uint64_t cache_key = 0;
  google::protobuf::Arena arena;
  starlark::vm::frame* compatibility_frame = nullptr;
  bool init_done = false;
  const starlark::bytecode::Program* program = nullptr;
  starlark::vm::module_loader* starlark_loader = nullptr;
  const starlark::runtime::runtime_options* runtime_options = nullptr;
  std::string module_name;
  bool inner_module = false;
  starlark::runtime::context* native_ctx = nullptr;
  std::map<native_fn_stack_key, int> fns_in_stack;
  int32_t error_block_ptr = 0;
  int32_t error_ip = 0;
  std::vector<native_exec_context*> active_exec_stack;
  std::unique_ptr<native_exec_context> exec_context;
  std::vector<starlark::runtime::starlark_obj*> stack_buffer;
  std::vector<starlark::runtime::starlark_obj*> const_strings;
  std::vector<native_function_frame_cache> function_frame_cache;
  uint32_t max_stack_depth = 0;
  starlark::vm::module_metadata init_metadata;

  void prepare_rerun();
  void materialize_const_strings();
};

struct active_exec_stack_guard {
  module_runtime_state* mod_state = nullptr;
  native_exec_context* exec = nullptr;

  explicit active_exec_stack_guard(module_runtime_state* mod_state_, native_exec_context* exec_)
      : mod_state(mod_state_), exec(exec_) {
    if (mod_state != nullptr && exec != nullptr) {
      mod_state->active_exec_stack.push_back(exec);
    }
  }

  ~active_exec_stack_guard() {
    if (mod_state == nullptr || exec == nullptr) {
      return;
    }
    auto& stack = mod_state->active_exec_stack;
    if (!stack.empty() && stack.back() == exec) {
      stack.pop_back();
    }
  }
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_MODULE_RUNTIME_STATE_HPP_
