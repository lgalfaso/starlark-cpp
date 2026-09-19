// Copyright 2026 Lucas Mirelmann

#ifndef RUNTIME_HEX_FORMAT_HPP_
#define RUNTIME_HEX_FORMAT_HPP_

#include <cstdint>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

inline constexpr char kHexDigitsLower[] = "0123456789abcdef";

inline void write_hex_lower(uint64_t value, char* out, int digit_count) {
  for (int i = digit_count - 1; i >= 0; --i) {
    out[i] = kHexDigitsLower[value & 0xf];
    value >>= 4;
  }
}

inline void write_hex16_lower(uint64_t value, char* out) {
  write_hex_lower(value, out, 16);
}

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_HEX_FORMAT_HPP_
