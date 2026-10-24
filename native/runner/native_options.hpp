// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_RUNNER_NATIVE_OPTIONS_HPP_
#define NATIVE_RUNNER_NATIVE_OPTIONS_HPP_

#include <string>

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct native_options {
  std::string cache_root;
  // Lower LLVM codegen effort for long-running fuzzing (less RAM and CPU per JIT).
  bool fast_jit = false;
  // After JIT-compiling a module miss, emit a dylib under cache_root for later load_cached hits.
  bool write_back_dylib = false;
  // Re-execute module init (block 0) even when JIT module state is cached.
  bool rerun_module = false;
  // Populate the on-disk dylib cache (implies write_back_dylib for JIT misses).
  bool warm_cache = false;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_RUNNER_NATIVE_OPTIONS_HPP_
