// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_SIPHASH_HPP_
#define RUNTIME_SIPHASH_HPP_

#include <cstdint>

namespace starlark {
namespace runtime {

uint64_t siphash(const char *input, const size_t inlen, uint64_t k0, uint64_t k1);

}  // namespace runtime
}  // namespace starlark

#endif  // RUNTIME_SIPHASH_HPP_

