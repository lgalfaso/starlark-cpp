// Copyright 2026 Lucas Mirelmann

#include <iostream>
#include <string_view>

#include "bigint/number.hpp"

using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;

namespace {

void PerformDivision(std::string_view str1, std::string_view str2) {
  number n1 = parse_number(str1, nullptr, 0);
  number n2 = parse_number(str2, nullptr, 0);

  // Exclude division by zero
  if (n2 != number::zero()) {
    auto [result, remainder] = number::div(n1, n2);

    if ((result * n2) + remainder != n1) {
      std::cout << "A != B*C + D" << std::endl;
      std::cout << "A (n1) = " << n1.hex() << std::endl;
      std::cout << "B (n2) = " << n2.hex() << std::endl;
      __builtin_trap();  // Bug found! Invariant violated.
    }

    if (n2.abs_cmp(remainder) <= 0) {
      std::cout << "abs(C) <= abc(D)" << std::endl;
      std::cout << "A (n1) = " << n1.hex() << std::endl;
      std::cout << "B (n2) = " << n2.hex() << std::endl;
      __builtin_trap();  // Bug found! Invariant violated.
    }
  }
}

void NumberDivFuzzing_1(const char* data, size_t size) {
  if (size < 2) {
    return;
  }

  // Split data in half to create two strings
  size_t half = size / 2;
  std::string_view str1(data, half);
  std::string_view str2(data + half, size - half);

  PerformDivision(str1, str2);
}

void NumberDivFuzzing_2(const char* data, size_t size) {
  if (size < 4) {
    return;
  }

  // Split data in half to create two strings
  size_t three_forth = size * 3 / 4;
  std::string_view str1(data, three_forth);
  std::string_view str2(data + three_forth, size - three_forth);

  PerformDivision(str1, str2);
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  NumberDivFuzzing_1(reinterpret_cast<const char*>(data), size);
  NumberDivFuzzing_2(reinterpret_cast<const char*>(data), size);
  return 0;
}
