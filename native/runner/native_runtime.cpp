// Copyright 2026 Lucas Mirelmann

#include "native/runner/native_runtime.hpp"

#include <format>
#include <set>
#include <vector>
#include <memory>
#include <string>
#include <utility>

#include "native/exec/native_exec_context.hpp"
#include "proto/starlark_bytecode.pb.h"

namespace starlark {
namespace native {

std::string native_runtime::bytecode_cache_key(std::string_view module_name, std::string_view source) {
  return std::format("{}\x1f{}", module_name, source);
}

native_runtime::native_runtime() = default;

native_runtime::~native_runtime() = default;

module_runtime_state* native_runtime::find_state(uint64_t cache_key) const {
  auto it = states_.find(cache_key);
  if (it == states_.end()) {
    return nullptr;
  }
  return it->second.get();
}

module_runtime_state& native_runtime::emplace_state(uint64_t cache_key, std::unique_ptr<module_runtime_state> state) {
  if (auto* existing = find_state(cache_key)) {
    return *existing;
  }
  auto& ref = *state;
  if (ref.exec_context == nullptr) {
    ref.exec_context = std::make_unique<native_exec_context>();
  }
  states_[cache_key] = std::move(state);
  return ref;
}

void native_runtime::discard_state(uint64_t cache_key) {
  states_.erase(cache_key);
}

void native_runtime::discard_nested_module_states(uint64_t keep_main_cache_key, uint64_t keep_builtin_cache_key) {
  std::vector<uint64_t> to_discard;
  to_discard.reserve(states_.size());
  for (auto& [cache_key, state] : states_) {
    if (state == nullptr || cache_key == keep_main_cache_key || cache_key == keep_builtin_cache_key) {
      continue;
    }
    to_discard.push_back(cache_key);
  }
  for (auto cache_key : to_discard) {
    discard_state(cache_key);
  }
}

void native_runtime::rebind_module_loaders(starlark::vm::module_loader& loader) {
  for (auto& [_, state] : states_) {
    if (state != nullptr) {
      state->starlark_loader = &loader;
    }
  }
}

void native_runtime::reset_transient_state() {
  std::set<native_exec_context*> active_execs;
  for (auto& [_, state] : states_) {
    if (state == nullptr) {
      continue;
    }
    for (auto* exec : state->active_exec_stack) {
      active_execs.insert(exec);
    }
  }
  for (auto& [_, state] : states_) {
    if (state == nullptr) {
      continue;
    }
    state->fns_in_stack.clear();
    state->error_block_ptr = 0;
    state->error_ip = 0;
    if (state->exec_context == nullptr) {
      continue;
    }
    auto* exec = state->exec_context.get();
    exec->failed = false;
    if (active_execs.contains(exec)) {
      continue;
    }
    exec->eval_stack.clear();
    exec->frame_chain.clear();
    exec->module_result_frame = nullptr;
  }
}

const starlark::bytecode::Program* native_runtime::find_bytecode(std::string_view module_name, std::string_view source) const {
  auto it = bytecode_cache_.find(bytecode_cache_key(module_name, source));
  if (it == bytecode_cache_.end() || it->second.program == nullptr) {
    return nullptr;
  }
  return it->second.program;
}

starlark::bytecode::Program* native_runtime::store_bytecode(std::string_view module_name,
    std::string_view source,
    std::unique_ptr<google::protobuf::Arena> arena,
    starlark::bytecode::Program* program) {
  if (program == nullptr || arena == nullptr) {
    return nullptr;
  }
  bytecode_cache_entry entry{
      .source = std::string{source},
      .arena = std::move(arena),
      .program = program,
  };
  auto [it, _] = bytecode_cache_.insert_or_assign(bytecode_cache_key(module_name, source), std::move(entry));
  return it->second.program;
}

}  // namespace native
}  // namespace starlark
