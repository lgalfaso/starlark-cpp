// Copyright 2024-2025 Lucas Mirelmann

#include "benchmark/benchmark.h"

#include "unicode/ucd_code_points.hpp"

namespace ucd_benchmark {

// NOLINTNEXTLINE(runtime/references)
static void BM_CheckForXids(benchmark::State& state) {
  for (auto _ : state) {
    for (int i = 0; i <= 0xFFFFF; ++i) {
      ucd::is_XID_Start(i);
    }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_CheckForXids);

// NOLINTNEXTLINE(runtime/references)
static void BM_CheckForXidc(benchmark::State& state) {
  for (auto _ : state) {
    for (int i = 0; i <= 0xFFFFF; ++i) {
      ucd::is_XID_Continue(i);
    }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_CheckForXidc);

// NOLINTNEXTLINE(runtime/references)
static void BM_CCC(benchmark::State& state) {
  for (auto _ : state) {
    for (int i = 0; i <= 0xFFFFF; ++i) {
      ucd::ccc(i);
    }
  }
}
// Register the function as a benchmark
BENCHMARK(BM_CCC);

}  // namespace ucd_benchmark

BENCHMARK_MAIN();
