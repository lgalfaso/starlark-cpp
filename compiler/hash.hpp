// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_HASH_HPP_
#define COMPILER_HASH_HPP_

#include <cstdint>

namespace starlark {
namespace compiler {

constexpr int hash_size = 61;
constexpr int64_t hash_mask = ((static_cast<int64_t>(1)) << hash_size) - 1;

}  // namespace compiler
}  // namespace starlark

#endif  // COMPILER_HASH_HPP_

