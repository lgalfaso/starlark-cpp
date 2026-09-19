// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_NATIVE_OPTIONS_HPP_
#define NATIVE_NATIVE_OPTIONS_HPP_

#include <string>

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct native_options {
  std::string cache_root;
  bool write_back_dylib = false;
  // Re-execute module init (block 0) even when JIT module state is cached.
  bool rerun_module = false;
  // Populate the on-disk dylib cache (implies write_back_dylib for JIT misses).
  bool warm_cache = false;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_NATIVE_OPTIONS_HPP_
