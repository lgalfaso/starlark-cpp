// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_NATIVE_RUNTIME_HPP_
#define NATIVE_NATIVE_RUNTIME_HPP_

#include <cstdint>

#include <map>
#include <memory>
#include <string>

#include "native/engine/orc_engine.hpp"
#include "google/protobuf/arena.h"
#include "proto/starlark_bytecode.pb.h"
#include "native/exec/module_runtime_state.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

class native_runtime {
 public:
  native_runtime();
  ~native_runtime();

  native_runtime(const native_runtime&) = delete;
  native_runtime& operator=(const native_runtime&) = delete;

  orc_engine& jit() { return engine_; }

  module_runtime_state* find_state(uint64_t cache_key) const;
  module_runtime_state& emplace_state(uint64_t cache_key, std::unique_ptr<module_runtime_state> state);
  void discard_state(uint64_t cache_key);
  void reset_transient_state();
  void discard_nested_module_states(uint64_t keep_main_cache_key, uint64_t keep_builtin_cache_key);
  void rebind_module_loaders(starlark::vm::module_loader& loader);

  const starlark::bytecode::Program* find_bytecode(std::string_view module_name, std::string_view source) const;
  starlark::bytecode::Program* store_bytecode(std::string_view module_name,
      std::string_view source,
      std::unique_ptr<google::protobuf::Arena> arena,
      starlark::bytecode::Program* program);

 private:
  static std::string bytecode_cache_key(std::string_view module_name, std::string_view source);

  struct bytecode_cache_entry {
    std::string source;
    std::unique_ptr<google::protobuf::Arena> arena;
    starlark::bytecode::Program* program = nullptr;
  };

  orc_engine engine_;
  std::map<uint64_t, std::unique_ptr<module_runtime_state>> states_;
  std::map<std::string, bytecode_cache_entry, std::less<>> bytecode_cache_;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_NATIVE_RUNTIME_HPP_
