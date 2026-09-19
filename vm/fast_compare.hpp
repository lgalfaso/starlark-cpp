// Copyright 2026 Lucas Mirelmann

#ifndef VM_FAST_COMPARE_HPP_
#define VM_FAST_COMPARE_HPP_

#include <optional>

#include "runtime/object_kind.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

inline bool fast_equals(starlark::runtime::starlark_obj* lhs, starlark::runtime::starlark_obj* rhs) {
  if (auto fast = starlark::runtime::try_fast_equals(lhs, rhs)) {
    return *fast;
  }
  return lhs != nullptr && rhs != nullptr && lhs->equals(*rhs);
}

inline std::optional<int> fast_int_cmp(starlark::runtime::starlark_obj* lhs, starlark::runtime::starlark_obj* rhs) {
  if (lhs != nullptr && rhs != nullptr && lhs->kind() == starlark::runtime::object_kind::kInt &&
      rhs->kind() == starlark::runtime::object_kind::kInt) {
    const auto l = lhs->as_int64();
    const auto r = rhs->as_int64();
    if (l < r) {
      return -1;
    }
    if (l > r) {
      return 1;
    }
    return 0;
  }
  return std::nullopt;
}

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_FAST_COMPARE_HPP_
