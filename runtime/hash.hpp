// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_HASH_HPP_
#define RUNTIME_HASH_HPP_

#include <cstdint>

namespace starlark {
namespace runtime {

constexpr int hash_size = 61;
constexpr int64_t hash_mask = ((static_cast<int64_t>(1)) << hash_size) - 1;

}  // namespace runtime
}  // namespace starlark

#endif  // RUNTIME_HASH_HPP_

