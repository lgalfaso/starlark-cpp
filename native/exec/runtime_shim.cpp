// Copyright 2026 Lucas Mirelmann

#include "native/exec/runtime_shim.hpp"

#include <map>
#include <functional>
#include <string>

#include "google/protobuf/arena.h"
#include "native/exec/exec_error.hpp"
#include "native/exec/frame_setup_cache.hpp"
#include "native/exec/native_exec_context.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_object.hpp"
#include "vm/frame.hpp"
#include "vm/frame_factory.hpp"
#include "vm/module_loader.hpp"
#include "vm/module_metadata.hpp"
#include "vm/predeclared_context.hpp"

using context = ::starlark::runtime::context;
using error_fn = ::starlark::runtime::error_fn;
using frame = ::starlark::vm::frame;
using starlark_obj = ::starlark::runtime::starlark_obj;

namespace starlark {
namespace native {

namespace {

using ::starlark::bytecode::BlockType;
using ::starlark::vm::copy_frame_symbols_to_arena;
using ::starlark::vm::create_frame_on_arena;
using ::starlark::vm::frame_metadata;

const frame_metadata* find_init_frame(const module_runtime_state* state, BlockType block_type) {
  if (state == nullptr) {
    return nullptr;
  }
  return starlark::vm::frame_metadata_for(state->init_metadata, block_type);
}

std::map<std::string, starlark_obj*, std::less<>> build_predeclared(module_runtime_state* state, context& ctx) {
  std::map<std::string, starlark_obj*, std::less<>> global_context;
  starlark::vm::add_core_predeclared_globals(global_context, ctx);
  if (state != nullptr && state->starlark_loader != nullptr) {
    auto builtin = state->starlark_loader->load_module("@@//:builtin.star");
    if (builtin.ok() && (*builtin)->ready()) {
      auto* builtin_frame = (*builtin)->get().first;
      for (int i = 0; i < builtin_frame->names->size(); ++i) {
        const auto& name = builtin_frame->names->Get(i);
        if (name == "max" || name == "min" || name == "sorted") {
          global_context[name] = builtin_frame->elements[static_cast<std::size_t>(i)];
        }
      }
    }
  }
  if (state != nullptr && state->starlark_loader != nullptr && !state->module_name.empty()) {
    auto mod = state->starlark_loader->load_module(state->module_name);
    if (mod.ok()) {
      for (const auto& [name, value] : (*mod)->custom_binding()) {
        global_context[name] = value;
      }
    }
  }
  return global_context;
}

}  // namespace

extern "C" {

void starlark_obj_rt_ensure_stack_capacity(native_exec_context* exec, int32_t required);

void* starlark_rt_module_init_begin(module_runtime_state* state, error_fn* err) {
  if (state == nullptr || state->runtime_options == nullptr || state->exec_context == nullptr) {
    return nullptr;
  }
  state->native_ctx = google::protobuf::Arena::Create<context>(&state->arena, state->arena, *state->runtime_options);
  if (state->exec_context->runner_context != nullptr) {
    state->native_ctx->runner_context(state->exec_context->runner_context);
  }
  auto* exec = state->exec_context.get();
  exec->reset_for_module_init(state, state->native_ctx, err);
  state->materialize_const_strings();
  build_function_frame_cache(*state);
  state->fns_in_stack.clear();
  state->active_exec_stack.clear();
  state->active_exec_stack.push_back(exec);
  return exec;
}

bool starlark_rt_exec_jit_function_entry(native_exec_context* exec) {
  if (exec == nullptr || exec->mod == nullptr) {
    return false;
  }
  starlark_obj_rt_ensure_stack_capacity(exec, static_cast<int32_t>(exec->mod->max_stack_depth));
  return true;
}

bool starlark_rt_module_init_end(native_exec_context* exec) {
  if (exec == nullptr || exec->mod == nullptr) {
    return false;
  }
  if (exec->module_result_frame == nullptr) {
    exec->mod->active_exec_stack.clear();
    return false;
  }
  exec->mod->compatibility_frame = exec->module_result_frame;
  for (auto& element : exec->mod->compatibility_frame->elements) {
    if (element != nullptr) {
      element->freeze();
    }
  }
  exec->mod->init_done = true;
  exec->mod->active_exec_stack.clear();
  return true;
}

bool starlark_rt_exec_create_predeclared(native_exec_context* exec, module_runtime_state* state_ptr, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* state = exec->mod != nullptr ? exec->mod : state_ptr;
  if (state == nullptr) {
    fail_exec_invariant(exec, "module state must be available during predeclared frame creation");
    return false;
  }
  const frame_metadata* metadata = find_init_frame(state, BlockType::PREDECLARED_BLOCK);
  if (metadata == nullptr) {
    fail_exec_invariant(exec, "predeclared frame metadata must exist");
    return false;
  }
  auto* symbols = copy_frame_symbols_to_arena(ctx->arena(), *metadata);
  if (metadata->symbols.empty()) {
    native_frame_chain_push(exec, create_frame_on_arena(ctx->arena(), symbols));
    return true;
  }
  auto global_context = build_predeclared(state, *ctx);
  auto* global_frame = create_frame_on_arena(ctx->arena(), symbols);
  int count = 0;
  for (const auto& symbol : metadata->symbols) {
    auto pos = global_context.find(symbol);
    if (pos == global_context.end()) {
      report_exec_error(exec, err, "symbol not available");
      return false;
    }
    global_frame->elements[count++] = pos->second;
  }
  native_frame_chain_push(exec, global_frame);
  return true;
}

void starlark_rt_exec_create_frame_meta(native_exec_context* exec, std::uint32_t meta_index) {
  if (exec->mod == nullptr || meta_index >= exec->mod->init_metadata.frames.size()) {
    fail_exec_invariant(exec, "frame metadata index out of range");
    return;
  }
  const frame_metadata* metadata = &exec->mod->init_metadata.frames[meta_index];
  auto* ctx = effective_ctx(exec, nullptr);
  google::protobuf::Arena& arena = metadata->block_type == BlockType::MODULE_BLOCK ? exec->mod->native_ctx->arena() : ctx->arena();
  auto* symbols = copy_frame_symbols_to_arena(arena, *metadata);
  auto* frame = create_frame_on_arena(arena, symbols);
  if (metadata->block_type == BlockType::MODULE_BLOCK) {
    exec->module_result_frame = frame;
  }
  native_frame_chain_push(exec, frame);
}

bool starlark_rt_exec_setup_native_fn_frame(native_exec_context* exec,
    module_runtime_state* mod_state,
    int32_t block_idx,
    starlark_obj** argv,
    int32_t argc,
    bool from_eval_stack) {
  if (exec == nullptr || mod_state == nullptr) {
    return false;
  }
  return setup_native_fn_frame_for_invoke(*exec, *mod_state, block_idx, argv, argc, from_eval_stack);
}

bool starlark_rt_exec_setup_native_fn_frame_from_eval(native_exec_context* exec,
    module_runtime_state* mod_state,
    int32_t block_idx,
    int32_t argc) {
  return starlark_rt_exec_setup_native_fn_frame(exec, mod_state, block_idx, nullptr, argc, true);
}

bool starlark_rt_exec_setup_native_fn_frame_from_argv(native_exec_context* exec,
    module_runtime_state* mod_state,
    int32_t block_idx,
    starlark::runtime::starlark_obj** argv,
    int32_t argc) {
  return starlark_rt_exec_setup_native_fn_frame(exec, mod_state, block_idx, argv, argc, false);
}

}  // extern "C"

void retain_runtime_symbols_for_jit() {
  static void* const k_symbols[] = {
      reinterpret_cast<void*>(&starlark_rt_module_init_begin),
      reinterpret_cast<void*>(&starlark_rt_module_init_end),
      reinterpret_cast<void*>(&starlark_rt_exec_create_predeclared),
      reinterpret_cast<void*>(&starlark_rt_exec_create_frame_meta),
      reinterpret_cast<void*>(&starlark_rt_exec_setup_native_fn_frame_from_eval),
      reinterpret_cast<void*>(&starlark_rt_exec_setup_native_fn_frame_from_argv),
      reinterpret_cast<void*>(&starlark_rt_exec_jit_function_entry),
  };
  __attribute__((used)) static const void* const anchor = k_symbols;
  (void)anchor;
}

}  // namespace native
}  // namespace starlark
