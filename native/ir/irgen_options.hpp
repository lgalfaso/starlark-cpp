// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_IR_IRGEN_OPTIONS_HPP_
#define NATIVE_IR_IRGEN_OPTIONS_HPP_

#include <cstdint>

#include "vm/module_metadata.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct irgen_options {
  uint64_t cache_key = 0;
  uint32_t max_stack_depth = 16;
  const starlark::vm::module_metadata* metadata = nullptr;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_IR_IRGEN_OPTIONS_HPP_
