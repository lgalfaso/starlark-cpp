// Copyright 2026 Lucas Mirelmann

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "grammar/parsing_options.hpp"
#include "interpreter/interpreter.hpp"
#include "interpreter/interpreter_test_runner.hpp"
#include "interpreter/module_loader.hpp"
#include "io/read_file.hpp"
#include "logging/logging.hpp"
#include "runtime/options.hpp"
#include "runtime/parsing_options.hpp"
#include "runtime/starlark_object.hpp"

using ::starlark::grammar::get_parsing_options;
using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::interpreter;
using ::starlark::interpreter::kv_module_loader;
using ::starlark::interpreter_runner::split_test_case;
using ::starlark::logging::logger;
using ::starlark::logging::pretty_log;
using ::starlark::runtime::get_runtime_options;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::testing::SizeIs;

namespace {

TEST(Parser, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(3));

  // Read the two files.
  auto starlark_program = starlark::io::read_file(argv[1]);
  ASSERT_TRUE(starlark_program.has_value());
  auto expeted_error_message = starlark::io::read_file(argv[2]);
  ASSERT_TRUE(expeted_error_message.has_value());

  // Get the grammar and runtime options.
  grammar_options g_options = get_parsing_options(*starlark_program);
  runtime_options r_options = get_runtime_options(*starlark_program, std::cout);

  // Prepare the modules.
  auto modules_parts = split_test_case(*starlark_program);
  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  for (const auto& [module_name, source] : modules_parts) {
    modules.try_emplace(module_name, source, std::map<std::string, starlark_obj*, std::less<>>{});
  }
  kv_module_loader loader{modules};

  interpreter runner;
  logger logging;
  auto result = runner.run(loader, "main", g_options, r_options, logging);
  ASSERT_FALSE(result.ok());
  ASSERT_FALSE(logging.empty());
  // TODO(lmirelmann): We should check all the errors.
  EXPECT_EQ(pretty_log(*logging.begin()), *expeted_error_message);
}

}  // namespace

