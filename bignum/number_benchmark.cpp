// Copyright 2025 Lucas Mirelmann

#include "benchmark/benchmark.h"

#include "bignum/number.hpp"

using bignum::number;

namespace number_benchmark {

// NOLINTNEXTLINE(runtime/references)
static void BM_Karatsuba(benchmark::State& state) {
  const int length = 10000;
  std::string block = "1234567890abcdef";
  std::string hex_number;
  hex_number.reserve(length * block.size());
  for (int i = 0; i < length; ++i) {
    hex_number += block;
  }
  number num = number::parse_hex(hex_number);

  for (auto _ : state) {
    number n = num;
    n.karatsuba(num, state.range(0));
  }
}
// Register the function as a benchmark
BENCHMARK(BM_Karatsuba)->DenseRange(0, 64, 1)->DenseRange(64, 1024, 16)->DenseRange(1024, 8192, 64);

}  // namespace number_benchmark

BENCHMARK_MAIN();

