// Copyright 2026 Lucas Mirelmann

#include <filesystem>
#include <map>
#include <sstream>
#include <string>

#include "gtest/gtest.h"
#include "interpreter/interpreter.hpp"
#include "native/runner/native_options.hpp"
#include "native/runner/native_runner.hpp"
#include "native/runner/native_runtime.hpp"
#include "vm/module_loader.hpp"

using ::starlark::interpreter::interpreter;
using ::starlark::native::native_options;
using ::starlark::native::native_runner;
using ::starlark::native::native_runtime;
using ::starlark::vm::kv_module_loader;

namespace {

void run_both(const std::string& code, const std::string& module_name) {
  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark::runtime::starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(module_name, code, std::map<std::string, starlark::runtime::starlark_obj*, std::less<>>{});

  auto cache_root = std::filesystem::temp_directory_path() / "starlark-native-test" / module_name;
  std::error_code ec;
  std::filesystem::remove_all(cache_root, ec);

  kv_module_loader loader{modules};
  starlark::grammar::grammar_options g_options;
  starlark::runtime::runtime_options r_options;
  native_options n_options{.cache_root = cache_root.string()};
  starlark::logging::logger logging;

  interpreter interp;
  auto interp_result = interp.run(loader, module_name, g_options, r_options, logging);
  ASSERT_TRUE(interp_result.ok());

  kv_module_loader loader2{modules};
  native_runtime runtime;
  native_runner native{runtime};
  auto native_result = native.run(loader2, module_name, g_options, r_options, n_options, logging);
  ASSERT_TRUE(native_result.ok());
  EXPECT_EQ((*interp_result)->elements.size(), (*native_result)->elements.size());
}

}  // namespace

TEST(Conformance, DefModuleExports) {
  run_both(R"starlark(
def foo():
  return 1
)starlark",
      "test");
}

TEST(Conformance, FibonacciModule) {
  run_both(R"starlark(
def fibonacci(n):
  a, b = 0, 1
  for _ in range(n):
    a, b = b, a + b
  return a
)starlark",
      "fibonacci");
}

TEST(Conformance, FibonacciModuleInit) {
  run_both(R"starlark(
def fibonacci(n):
  a, b = 0, 1
  for _ in range(n):
    a, b = b, a + b
  return a
fibonacci(100)
)starlark",
      "fibonacci_init");
}
