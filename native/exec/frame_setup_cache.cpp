// Copyright 2026 Lucas Mirelmann

#include "native/exec/frame_setup_cache.hpp"

#include <algorithm>
#include <string>

#include "google/protobuf/arena.h"
#include "vm/frame_factory.hpp"

using starlark_obj = ::starlark::runtime::starlark_obj;

namespace starlark {
namespace native {

namespace {

using ::starlark::vm::copy_frame_symbols_to_arena;
using ::starlark::vm::create_frame_on_arena;
using ::starlark::vm::frame;
using ::starlark::vm::function_signature_metadata;

void build_positional_slots(native_function_frame_cache& cache, const function_signature_metadata& fn_meta) {
  cache.positional_slots.clear();
  cache.sequential_slots = true;
  cache.positional_slots.resize(static_cast<std::size_t>(fn_meta.positional_param_count));
  for (int i = 0; i < fn_meta.positional_param_count; ++i) {
    const int32_t slot = fn_meta.params[static_cast<std::size_t>(i)].pos_in_frame;
    cache.positional_slots[static_cast<std::size_t>(i)] = slot;
    if (slot != i) {
      cache.sequential_slots = false;
    }
  }
}

}  // namespace

void build_function_frame_cache(module_runtime_state& state) {
  state.function_frame_cache.clear();
  if (state.native_ctx == nullptr) {
    return;
  }
  google::protobuf::Arena& arena = state.native_ctx->arena();
  const auto& functions = state.init_metadata.functions;
  state.function_frame_cache.resize(functions.size());
  for (std::size_t fn_idx = 0; fn_idx < functions.size(); ++fn_idx) {
    auto& cache = state.function_frame_cache[fn_idx];
    const auto& fn_meta = functions[fn_idx];
    cache.names = copy_frame_symbols_to_arena(arena, fn_meta);
    build_positional_slots(cache, fn_meta);
  }
}

const google::protobuf::RepeatedPtrField<std::string>* function_frame_names(module_runtime_state& mod_state,
    std::size_t fn_idx,
    starlark::runtime::context& ctx,
    const function_signature_metadata& fn_meta) {
  if (fn_idx < mod_state.function_frame_cache.size() && mod_state.function_frame_cache[fn_idx].names != nullptr) {
    return mod_state.function_frame_cache[fn_idx].names;
  }
  return copy_frame_symbols_to_arena(ctx.arena(), fn_meta);
}

frame* acquire_function_frame(native_exec_context& exec, google::protobuf::Arena& arena, const google::protobuf::RepeatedPtrField<std::string>* names) {
  if (names != nullptr) {
    const std::size_t slot_count = static_cast<std::size_t>(names->size());
    if (slot_count < exec.frame_pool_by_slot_count.size()) {
      auto& pool = exec.frame_pool_by_slot_count[slot_count];
      if (!pool.empty()) {
        frame* reused = pool.back();
        pool.pop_back();
        reused->iterators.clear();
        reused->pinned_for_closure = false;
        std::fill(reused->elements.begin(), reused->elements.end(), nullptr);
        return reused;
      }
    }
  }
  return create_frame_on_arena(arena, names);
}

void recycle_top_function_frame(native_exec_context& exec) {
  if (exec.frame_chain.depth == 0) {
    return;
  }
  frame* frm = exec.frame_chain.back();
  if (frm == nullptr || frm->pinned_for_closure) {
    return;
  }
  const std::size_t slot_count = frm->elements.size();
  if (exec.frame_pool_by_slot_count.size() <= slot_count) {
    exec.frame_pool_by_slot_count.resize(slot_count + 1);
  }
  frm->iterators.clear();
  exec.frame_pool_by_slot_count[slot_count].push_back(frm);
}

void bind_positional_args_to_frame(frame& fn_frame,
    const function_signature_metadata& fn_meta,
    const native_function_frame_cache& cache,
    std::size_t fn_idx,
    starlark_obj** argv,
    int32_t argc) {
  if (cache.sequential_slots) {
    for (int i = 0; i < argc; ++i) {
      fn_frame.elements[static_cast<std::size_t>(i)] = argv[static_cast<std::size_t>(i)];
    }
    return;
  }
  if (static_cast<std::size_t>(argc) == cache.positional_slots.size()) {
    for (int i = 0; i < argc; ++i) {
      fn_frame.elements[static_cast<std::size_t>(cache.positional_slots[static_cast<std::size_t>(i)])] =
          argv[static_cast<std::size_t>(i)];
    }
    return;
  }
  (void)fn_idx;
  for (int i = 0; i < argc; ++i) {
    fn_frame.elements[fn_meta.params[static_cast<std::size_t>(i)].pos_in_frame] = argv[static_cast<std::size_t>(i)];
  }
}

void bind_positional_args_from_eval_stack(frame& fn_frame,
    native_exec_context& exec,
    const function_signature_metadata& fn_meta,
    const native_function_frame_cache& cache,
    std::size_t fn_idx,
    uint32_t callee_idx,
    int32_t argc) {
  if (cache.sequential_slots) {
    for (int i = 0; i < argc; ++i) {
      fn_frame.elements[static_cast<std::size_t>(i)] = exec.eval_stack.at(callee_idx + 1 + static_cast<uint32_t>(i));
    }
    return;
  }
  if (static_cast<std::size_t>(argc) == cache.positional_slots.size()) {
    for (int i = 0; i < argc; ++i) {
      fn_frame.elements[static_cast<std::size_t>(cache.positional_slots[static_cast<std::size_t>(i)])] =
          exec.eval_stack.at(callee_idx + 1 + static_cast<uint32_t>(i));
    }
    return;
  }
  (void)fn_idx;
  for (int i = 0; i < argc; ++i) {
    fn_frame.elements[fn_meta.params[static_cast<std::size_t>(i)].pos_in_frame] =
        exec.eval_stack.at(callee_idx + 1 + static_cast<uint32_t>(i));
  }
}

bool setup_native_fn_frame_for_invoke(native_exec_context& exec,
    module_runtime_state& mod_state,
    int32_t block_idx,
    starlark_obj** argv,
    int32_t argc,
    bool from_eval_stack) {
  if (block_idx <= 0) {
    return false;
  }
  const auto fn_idx = static_cast<std::size_t>(block_idx - 1);
  if (fn_idx >= mod_state.init_metadata.functions.size()) {
    return false;
  }
  const auto& fn_meta = mod_state.init_metadata.functions[fn_idx];
  if (argc != fn_meta.positional_param_count) {
    return false;
  }
  auto* ctx = effective_ctx(&exec, nullptr);
  const native_function_frame_cache& cache =
      fn_idx < mod_state.function_frame_cache.size() ? mod_state.function_frame_cache[fn_idx] : native_function_frame_cache{};
  auto* symbols = function_frame_names(mod_state, fn_idx, *ctx, fn_meta);
  auto* fn_frame = acquire_function_frame(exec, ctx->arena(), symbols);
  if (from_eval_stack) {
    const uint32_t callee_idx = exec.eval_stack.size - static_cast<uint32_t>(argc) - 1;
    bind_positional_args_from_eval_stack(*fn_frame, exec, fn_meta, cache, fn_idx, callee_idx, argc);
    exec.eval_stack.resize(callee_idx);
  } else {
    bind_positional_args_to_frame(*fn_frame, fn_meta, cache, fn_idx, argv, argc);
  }
  native_frame_chain_push(&exec, fn_frame);
  return true;
}

}  // namespace native
}  // namespace starlark
