// Copyright 2026 Lucas Mirelmann

#include <unistd.h>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>  // NOLINT(build/c++17)
#include <format>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "grammar/parsing_options.hpp"
#include "io/read_file.hpp"
#include "logging/logging.hpp"
#include "native/runner/native_options.hpp"
#include "native/runner/native_runner.hpp"
#include "runtime/parsing_options.hpp"
#include "runtime/starlark_object.hpp"
#include "vm/module_loader.hpp"
#include "vm/test_case.hpp"

using ::starlark::grammar::get_parsing_options;
using ::starlark::grammar::grammar_options;
using ::starlark::logging::logger;
using ::starlark::logging::pretty_log;
using ::starlark::native::native_options;
using ::starlark::native::native_runner;
using ::starlark::native::native_runtime;
using ::starlark::runtime::get_runtime_options;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::starlark::vm::kv_module_loader;
using ::starlark::vm::test::split_test_case;
using ::testing::SizeIs;

namespace {

std::filesystem::path native_test_cache_root() {
  return std::filesystem::temp_directory_path() / std::format("starlark-native-test-{}", ::getpid());
}

TEST(Errors, NativeTestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(3));

  auto starlark_program = starlark::io::read_file(argv[1]);
  ASSERT_TRUE(starlark_program.has_value());
  auto expected_error_message = starlark::io::read_file(argv[2]);
  ASSERT_TRUE(expected_error_message.has_value());

  grammar_options g_options = get_parsing_options(*starlark_program);
  runtime_options r_options = get_runtime_options(*starlark_program, std::cout);

  auto modules_parts = split_test_case(*starlark_program);
  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  for (const auto& [module_name, source] : modules_parts) {
    modules.try_emplace(module_name, source, std::map<std::string, starlark_obj*, std::less<>>{});
  }
  kv_module_loader loader{modules};

  logger logging;
  native_runtime runtime;
  native_runner runner(runtime);
  native_options n_options{.cache_root = native_test_cache_root().string()};
  auto result = runner.run(loader, "main", g_options, r_options, n_options, logging);
  runtime.reset_transient_state();
  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(logging.empty());
  // TODO(lmirelmann): We should check all the errors.
  EXPECT_EQ(pretty_log(*logging.begin()), *expected_error_message);
}

}  // namespace
