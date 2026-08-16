// Copyright 2026 Lucas Mirelmann

#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <format>
#include <map>
#include <string>
#include <utility>

#include "interpreter/interpreter.hpp"
#include "third-party/defer.hpp"

using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::interpreter;
using ::starlark::interpreter::kv_module_loader;
using ::starlark::logging::logger;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;

void run_code(benchmark::State& state, const char* file_name, std::string_view module_name) {
  std::string starlark_code;
  {
    int starlark_fd = open(file_name, O_RDONLY);
    if (starlark_fd == 0) {
      state.SkipWithError(std::format("Unable to open {}", file_name));
      return;
    }
    defer { close(starlark_fd); };
    struct stat sb;
    if (fstat(starlark_fd, &sb) < 0) {
      state.SkipWithError(std::format("Unable to read metadata on {}", file_name));
      return;
    }
    starlark_code.resize(sb.st_size);
    read(starlark_fd, starlark_code.data(), sb.st_size);
  }

  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(std::string{module_name}, starlark_code, std::map<std::string, starlark_obj*, std::less<>>{});
  for (auto _ : state) {
    std::basic_ostringstream<char> out;
    kv_module_loader loader{modules};

    interpreter runner;
    logger logging;

    runner.run(loader, module_name, grammar_options{}, runtime_options{.out = out}, logging);
  }
}

// NOLINTNEXTLINE(runtime/references)
static void BM_InterpreterBenchBase64(benchmark::State& state) {
  run_code(state, "bench/base64.star", "base64");
}
BENCHMARK(BM_InterpreterBenchBase64)->Name("base64");

// NOLINTNEXTLINE(runtime/references)
static void BM_InterpreterBenchBTree(benchmark::State& state) {
  run_code(state, "bench/btree.star", "btree");
}
BENCHMARK(BM_InterpreterBenchBTree)->Name("btree");

// NOLINTNEXTLINE(runtime/references)
static void BM_InterpreterBenchFibonacci(benchmark::State& state) {
  run_code(state, "bench/fibonacci.star", "fibonacci");
}
BENCHMARK(BM_InterpreterBenchFibonacci)->Name("fibonacci");

// NOLINTNEXTLINE(runtime/references)
static void BM_InterpreterBenchMandelbrot(benchmark::State& state) {
  run_code(state, "bench/mandelbrot.star", "mandelbrot");
}
BENCHMARK(BM_InterpreterBenchMandelbrot)->Name("mandelbrot");

// NOLINTNEXTLINE(runtime/references)
static void BM_InterpreterBenchNQueens(benchmark::State& state) {
  run_code(state, "bench/nqueens.star", "nqueens");
}
BENCHMARK(BM_InterpreterBenchNQueens)->Name("nqueens");

// NOLINTNEXTLINE(runtime/references)
static void BM_InterpreterBenchE(benchmark::State& state) {
  run_code(state, "bench/e.star", "e");
}
BENCHMARK(BM_InterpreterBenchE)->Name("e");

BENCHMARK_MAIN();

