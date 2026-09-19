// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_RUNNER_STATE_HPP_
#define NATIVE_RUNNER_STATE_HPP_

#include <string>

#include "native/runner/native_options.hpp"
#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

class native_runtime;

struct native_runner_state {
  native_runtime* runtime = nullptr;
  starlark::vm::module_loader* loader = nullptr;
  const native_options* options = nullptr;
  uint64_t main_module_cache_key = 0;
  bool has_main_module_cache_key = false;
  uint64_t builtin_module_cache_key = 0;
  bool has_builtin_module_cache_key = false;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_RUNNER_STATE_HPP_
