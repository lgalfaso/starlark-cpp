// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_SIPHASH_HPP_
#define COMPILER_SIPHASH_HPP_

#include <cstdint>

namespace starlark {
namespace compiler {

uint64_t siphash(const char *input, const size_t inlen, uint64_t k0, uint64_t k1);

}  // namespace compiler
}  // namespace starlark

#endif  // COMPILER_SIPHASH_HPP_

