// Copyright 2026 Lucas Mirelmann

#include <benchmark/benchmark.h>

#include <format>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "grammar/options.hpp"
#include "interpreter/interpreter.hpp"
#include "io/read_file.hpp"
#include "vm/module_loader.hpp"

using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::starlark::vm::kv_module_loader;

namespace {

std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> make_modules(const char* file_name, std::string_view module_name) {
  auto starlark_code = starlark::io::read_file(file_name);
  if (!starlark_code) {
    return {};
  }
  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(std::string{module_name}, *starlark_code, std::map<std::string, starlark_obj*, std::less<>>{});
  return modules;
}

void run_interpreter_code(benchmark::State& state, const char* file_name, std::string_view module_name) {
  auto modules = make_modules(file_name, module_name);
  if (modules.empty()) {
    state.SkipWithError(std::format("Unable to read {}", file_name));
    return;
  }

  interpreter runner;
  for (auto _ : state) {
    std::basic_ostringstream<char> out;
    kv_module_loader loader{modules};
    logger logging;
    runner.run(loader, module_name, grammar_options{}, runtime_options{.out = out}, logging);
  }
}

}  // namespace

#define STARLARK_BENCH(name, file, module)                                                              \
  static void BM_##name##_Interpreter(benchmark::State& state) {                                        \
    run_interpreter_code(state, file, module);                                                          \
  }                                                                                                     \
  BENCHMARK(BM_##name##_Interpreter)->Name(#name "/interpreter");

STARLARK_BENCH(base64, "bench/base64.star", "base64")
STARLARK_BENCH(btree, "bench/btree.star", "btree")
STARLARK_BENCH(fibonacci, "bench/fibonacci.star", "fibonacci")
STARLARK_BENCH(mandelbrot, "bench/mandelbrot.star", "mandelbrot")
STARLARK_BENCH(nqueens, "bench/nqueens.star", "nqueens")
STARLARK_BENCH(e, "bench/e.star", "e")

BENCHMARK_MAIN();
