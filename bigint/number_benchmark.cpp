// Copyright 2025-2026 Lucas Mirelmann

#include <string>

#include "benchmark/benchmark.h"

#include "bigint/number.hpp"

using ::starlark::bigint::number;

namespace starlark {
namespace number_benchmark {

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
BENCHMARK(BM_NumberMultShort)->RangeMultiplier(2)->Range(1<<4, 1<<22)
    ->Complexity();

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
BENCHMARK(BM_NumberMult)->RangeMultiplier(2)->Range(1<<4, 1<<22)
    ->Complexity();

}  // namespace number_benchmark
}  // namespace starlark

BENCHMARK_MAIN();

