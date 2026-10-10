// Copyright 2026 Lucas Mirelmann

#include "native/exec/native_function.hpp"

#include <map>
#include <vector>
#include <string>
#include <utility>

#include "errors/runtime_error_messages.hpp"
#include "errors/source_highlight.hpp"
#include "google/protobuf/arena.h"
#include "native/abi/starlark_module_abi.hpp"
#include "native/exec/exec_error.hpp"
#include "native/exec/native_exec_context.hpp"
#include "native/exec/frame_setup_cache.hpp"
#include "native/exec/runtime_shim.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"
#include "vm/frame_factory.hpp"
#include "vm/function_call_binding.hpp"
#include "vm/module_metadata.hpp"

using frame = ::starlark::vm::frame;

namespace starlark {
namespace native {

namespace {

using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::vm::finish_bound_function_frame;
using ::starlark::vm::function_call_binding_state;
using ::starlark::vm::function_metadata_for_block;
using ::starlark::vm::function_signature_metadata;

struct fn_stack_guard {
  module_runtime_state* mod_state;
  native_fn_stack_key key;
  explicit fn_stack_guard(module_runtime_state* mod_state_, native_fn_stack_key key_) : mod_state(mod_state_), key(key_) {}
  ~fn_stack_guard() {
    if (mod_state == nullptr) {
      return;
    }
    auto it = mod_state->fns_in_stack.find(key);
    if (it != mod_state->fns_in_stack.end() && --it->second == 0) {
      mod_state->fns_in_stack.erase(it);
    }
  }
};

bool is_simple_positional_call(const function_signature_metadata& fn_meta,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args) {
  if (!named_args.empty()) {
    return false;
  }
  if (fn_meta.has_star_argument || fn_meta.has_star_star_argument) {
    return false;
  }
  if (fn_meta.keyword_only_parameter_count > 0) {
    return false;
  }
  return pos_args.size() == static_cast<std::size_t>(fn_meta.param_count);
}

}  // namespace

native_starlark_function::native_starlark_function(void* code_ptr,
    int32_t block_idx,
    std::string_view fn_name,
    std::string_view module_name,
    module_runtime_state* mod_state,
    std::vector<starlark_obj*> default_arguments,
    std::vector<frame*> closure_frames,
    bool inner_fn) :
    starlark_function(std::string{fn_name}, std::string{module_name}, starlark::runtime::object_kind::kFunction),
    code_ptr_(code_ptr),
    block_idx_(block_idx),
    mod_state_(mod_state),
    inner_(inner_fn),
    default_arguments_(std::move(default_arguments)),
    closure_frames_(std::move(closure_frames)) {
  if (mod_state_ == nullptr) {
    return;
  }
  const auto* fn_meta = function_metadata_for_block(mod_state_->init_metadata, block_idx_);
  if (fn_meta == nullptr) {
    return;
  }
  direct_dispatch_eligible_ =
      !fn_meta->has_star_argument && !fn_meta->has_star_star_argument && fn_meta->keyword_only_parameter_count == 0;
  for (std::size_t i = 0; i < fn_meta->params.size(); ++i) {
    named_argument_index_[fn_meta->params[i].name] = static_cast<int>(i);
  }
}

native_invoke_result native_starlark_function::invoke_jit(native_exec_context* exec,
    starlark_obj** argv,
    int32_t argc,
    bool args_on_eval_stack,
    context* ctx,
    error_fn* err,
    bool manage_recursion_stack) {
  native_invoke_result result;
  if (exec == nullptr || ctx == nullptr || err == nullptr) {
    result.failed = true;
    return result;
  }
  if (mod_state_ == nullptr || mod_state_->program == nullptr) {
    fail_exec_invariant(exec, "native function has no module state");
    result.failed = true;
    return result;
  }
  void* code_ptr = code_ptr_;
  if (code_ptr == nullptr) {
    fail_exec_invariant(exec, "native function has no code pointer");
    result.failed = true;
    return result;
  }
  const native_fn_stack_key stack_key{mod_state_->program, block_idx_};
  if (manage_recursion_stack) {
    if (!inner_ && !ctx->options().allow_recursion) {
      auto it = mod_state_->fns_in_stack.find(stack_key);
      if (it != mod_state_->fns_in_stack.end() && it->second > 0) {
        report_exec_error(exec, err, starlark::error_messages::error_v2_recursive_call(fn_name));
        result.failed = true;
        return result;
      }
    }
    mod_state_->fns_in_stack[stack_key]++;
  }

  const uint32_t frame_depth_before = exec->frame_chain.depth;
  for (auto* closure_frame : closure_frames_) {
    native_frame_chain_push(exec, closure_frame);
  }

  uint32_t callee_idx = 0;
  if (args_on_eval_stack) {
    if (exec->eval_stack.size < static_cast<uint32_t>(argc) + 1) {
      exec->frame_chain.resize(frame_depth_before);
      if (manage_recursion_stack) {
        auto it = mod_state_->fns_in_stack.find(stack_key);
        if (it != mod_state_->fns_in_stack.end() && --it->second == 0) {
          mod_state_->fns_in_stack.erase(it);
        }
      }
      result.failed = true;
      return result;
    }
    callee_idx = exec->eval_stack.size - static_cast<uint32_t>(argc) - 1;
  }

  if (args_on_eval_stack) {
    if (!setup_native_fn_frame_for_invoke(*exec, *mod_state_, block_idx_, nullptr, argc, true)) {
      exec->frame_chain.resize(frame_depth_before);
      if (manage_recursion_stack) {
        auto it = mod_state_->fns_in_stack.find(stack_key);
        if (it != mod_state_->fns_in_stack.end() && --it->second == 0) {
          mod_state_->fns_in_stack.erase(it);
        }
      }
      if (args_on_eval_stack) {
        exec->eval_stack.resize(callee_idx + 1);
        exec->eval_stack.data[callee_idx] = nullptr;
      }
      result.failed = true;
      return result;
    }
  } else if (argc > 0) {
    if (!setup_native_fn_frame_for_invoke(*exec, *mod_state_, block_idx_, argv, argc, false)) {
      exec->frame_chain.resize(frame_depth_before);
      if (manage_recursion_stack) {
        auto it = mod_state_->fns_in_stack.find(stack_key);
        if (it != mod_state_->fns_in_stack.end() && --it->second == 0) {
          mod_state_->fns_in_stack.erase(it);
        }
      }
      result.failed = true;
      return result;
    }
  }

  module_runtime_state* saved_code_mod = exec->code_mod;
  exec->code_mod = mod_state_;
  auto* fn = reinterpret_cast<native_fn_t>(code_ptr);
  starlark_obj* value = fn(exec, nullptr, 0, ctx, err);
  exec->code_mod = saved_code_mod;

  recycle_top_function_frame(*exec);
  exec->frame_chain.resize(frame_depth_before);
  if (manage_recursion_stack) {
    auto it = mod_state_->fns_in_stack.find(stack_key);
    if (it != mod_state_->fns_in_stack.end() && --it->second == 0) {
      mod_state_->fns_in_stack.erase(it);
    }
  }

  if (args_on_eval_stack) {
    exec->eval_stack.resize(callee_idx + 1);
    if (value == nullptr || exec->failed) {
      fail_exec_on_runtime_error(exec, err);
      exec->eval_stack.data[callee_idx] = nullptr;
      result.failed = true;
      return result;
    }
    exec->eval_stack.data[callee_idx] = value;
  } else if (value == nullptr || exec->failed) {
    fail_exec_on_runtime_error(exec, err);
    result.failed = true;
    return result;
  }

  result.value = value;
  result.failed = exec->failed;
  return result;
}

starlark::runtime::starlark_obj* native_starlark_function::call(const starlark::runtime::starlark_obj::pos_args_t& pos_args,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback) {
  if (mod_state_ == nullptr || mod_state_->program == nullptr) {
    error_callback.add_error("native function has no code pointer");
    return nullptr;
  }
  const auto* fn_meta_ptr = function_metadata_for_block(mod_state_->init_metadata, block_idx_);
  if (fn_meta_ptr == nullptr) {
    error_callback.add_error("native function has no metadata");
    return nullptr;
  }
  const auto& fn_meta = *fn_meta_ptr;

  void* code_ptr = code_ptr_;
  if (code_ptr == nullptr) {
    error_callback.add_error("native function has no code pointer");
    return nullptr;
  }
  const native_fn_stack_key stack_key{mod_state_->program, block_idx_};
  if (!inner_ && !ctx.options().allow_recursion) {
    auto it = mod_state_->fns_in_stack.find(stack_key);
    if (it != mod_state_->fns_in_stack.end() && it->second > 0) {
      error_callback.add_error(starlark::error_messages::error_v2_recursive_call(fn_name));
      return nullptr;
    }
  }
  mod_state_->fns_in_stack[stack_key]++;
  fn_stack_guard stack_guard(mod_state_, stack_key);

  if (is_simple_positional_call(fn_meta, pos_args, named_args)) {
    std::vector<starlark_obj*> argv(pos_args.begin(), pos_args.end());
    const int32_t argc = static_cast<int32_t>(argv.size());

    native_exec_context* reuse_exec = nullptr;
    if (!mod_state_->active_exec_stack.empty()) {
      reuse_exec = mod_state_->active_exec_stack.back();
    }

    if (reuse_exec != nullptr) {
      auto invoke_result = invoke_jit(reuse_exec, argv.data(), argc, false, &ctx, &error_callback, false);
      if (invoke_result.failed) {
        return nullptr;
      }
      return invoke_result.value;
    }

    native_exec_context exec;
    exec_error_bridge bridged_err(&exec, &error_callback);
    exec.reset_for_function_call(mod_state_, &ctx, &bridged_err, {});
    active_exec_stack_guard exec_guard(mod_state_, &exec);
    auto invoke_result = invoke_jit(&exec, argv.data(), argc, false, &ctx, &bridged_err, false);
    if (invoke_result.failed) {
      return nullptr;
    }
    return invoke_result.value;
  }

  native_exec_context* parent_exec = nullptr;
  std::vector<starlark_obj*> saved_parent_stack;
  if (!mod_state_->active_exec_stack.empty()) {
    parent_exec = mod_state_->active_exec_stack.back();
    saved_parent_stack.assign(parent_exec->eval_stack.data, parent_exec->eval_stack.data + parent_exec->eval_stack.size);
  }

  native_exec_context exec;
  exec_error_bridge bridged_err(&exec, &error_callback);
  exec.reset_for_function_call(mod_state_, &ctx, &bridged_err, closure_frames_);
  active_exec_stack_guard exec_guard(mod_state_, &exec);

  const auto fn_idx = static_cast<std::size_t>(block_idx_ - 1);
  auto* symbols = function_frame_names(*mod_state_, fn_idx, ctx, fn_meta);
  auto* fn_frame = acquire_function_frame(exec, ctx.arena(), symbols);
  function_call_binding_state binding_state;
  if (!starlark::vm::bind_function_arguments(fn_meta,
          named_argument_index_,
          default_arguments_,
          fn_frame->elements,
          pos_args,
          named_args,
          binding_state,
          error_callback)) {
    return nullptr;
  }
  finish_bound_function_frame(fn_meta, ctx.arena(), fn_frame->elements, binding_state, error_callback);

  native_frame_chain_push(&exec, fn_frame);
  auto* fn = reinterpret_cast<native_fn_t>(code_ptr);
  auto* result = fn(&exec, nullptr, 0, &ctx, &bridged_err);
  if (parent_exec != nullptr) {
    parent_exec->eval_stack.resize(static_cast<uint32_t>(saved_parent_stack.size()));
    for (std::size_t i = 0; i < saved_parent_stack.size(); ++i) {
      parent_exec->eval_stack.data[i] = saved_parent_stack[i];
    }
  }
  if (exec.failed) {
    return nullptr;
  }
  return result;
}

bool native_starlark_function::inner_equals(starlark::runtime::equals_comparator& comp, const starlark::runtime::starlark_obj* other) const {
  (void)comp;
  if (!same_starlark_type(other->kind(), kind())) {
    return false;
  }
  const auto* rhs = static_cast<const native_starlark_function*>(other);
  if (block_idx_ != rhs->block_idx_) {
    return false;
  }
  if (mod_state_ == nullptr || rhs->mod_state_ == nullptr || mod_state_->program != rhs->mod_state_->program) {
    return false;
  }
  if (closure_frames_.size() != rhs->closure_frames_.size()) {
    return false;
  }
  for (std::size_t i = 0; i < closure_frames_.size(); ++i) {
    if (closure_frames_[i] != rhs->closure_frames_[i]) {
      return false;
    }
  }
  return true;
}

void native_starlark_function::inner_cmp(starlark::runtime::order_comparator& comp,
    const starlark::runtime::starlark_obj* other,
    std::string_view op,
    bool extended,
    starlark::runtime::error_fn& error_callback) const {
  if (extended && same_starlark_type(kind(), other->kind()) && equals(*other)) {
    return;
  }
  starlark_function::inner_cmp(comp, other, op, extended, error_callback);
}

void native_starlark_function::inner_freeze(std::vector<starlark::runtime::starlark_obj*>& to_freeze) {
  for (auto* frame : closure_frames_) {
    for (auto* element : frame->elements) {
      if (element != nullptr) {
        to_freeze.push_back(element);
      }
    }
  }
  for (auto* value : default_arguments_) {
    if (value != nullptr) {
      to_freeze.push_back(value);
    }
  }
}

}  // namespace native
}  // namespace starlark
