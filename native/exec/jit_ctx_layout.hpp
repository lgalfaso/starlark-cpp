// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_EXEC_JIT_CTX_LAYOUT_HPP_
#define NATIVE_EXEC_JIT_CTX_LAYOUT_HPP_

#include <cstddef>
#include <cstdint>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct jit_ctx_layout {
  static std::size_t offsetof_small_integers() {
    return offsetof(starlark::runtime::context, small_integers);
  }
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_EXEC_JIT_CTX_LAYOUT_HPP_
