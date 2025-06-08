// Copyright 2025 Lucas Mirelmann

#include <string>

#include "benchmark/benchmark.h"

#include "bigint/number.hpp"

using starlark::bigint::number;

namespace starlark {
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
BENCHMARK(BM_Karatsuba)->DenseRange(0, 64, 1)->DenseRange(64, 1024, 16)
    ->DenseRange(1024, 8192, 64);


// NOLINTNEXTLINE(runtime/references)
static void BM_NumberMultShort(benchmark::State& state) {
  size_t size = state.range(0);
  const std::string to_parse(size, '7');
  number number_0 = number::parse_hex(to_parse);
  number number_1 = number::parse_hex("1234567890abcdef");
  for (auto _ : state) {
    number_0 * number_1;
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_NumberMultShort)
    ->RangeMultiplier(2)->Range(1<<4, 1<<22)->Complexity();

// NOLINTNEXTLINE(runtime/references)
static void BM_NumberMult(benchmark::State& state) {
  size_t size = state.range(0);
  const std::string to_parse(size, '7');
  number number_0 = number::parse_hex(to_parse);
  number number_1 = number::parse_hex(to_parse);
  for (auto _ : state) {
    number_0 * number_1;
  }
  state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_NumberMult)
    ->RangeMultiplier(2)->Range(1<<4, 1<<22)->Complexity();

}  // namespace number_benchmark
}  // namespace starlark

BENCHMARK_MAIN();

