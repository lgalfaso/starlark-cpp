// Copyright 2026 Lucas Mirelmann

#include <fcntl.h>

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
#include "logging/logging.hpp"
#include "runtime/options.hpp"
#include "runtime/parsing_options.hpp"
#include "runtime/starlark_object.hpp"
#include "third-party/defer.hpp"

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
  std::string starlark_program;
  {
    int starlark_fd = open(argv[1].c_str(), O_RDONLY);
    ASSERT_GT(starlark_fd, 0);
    defer { close(starlark_fd); };
    struct stat sb;
    ASSERT_GE(fstat(starlark_fd, &sb), 0);
    starlark_program.resize(sb.st_size);
    read(starlark_fd, starlark_program.data(), sb.st_size);
  }
  std::string expeted_error_message;
  {
    int error_message_fd = open(argv[2].c_str(), O_RDONLY);
    ASSERT_GT(error_message_fd, 0);
    defer { close(error_message_fd); };
    struct stat sb;
    ASSERT_GE(fstat(error_message_fd, &sb), 0);
    expeted_error_message.resize(sb.st_size);
    read(error_message_fd, expeted_error_message.data(), sb.st_size);
  }

  // Get the grammar and runtime options.
  grammar_options g_options = get_parsing_options(starlark_program);
  runtime_options r_options = get_runtime_options(starlark_program, std::cout);

  // Prepare the modules.
  auto modules_parts = split_test_case(starlark_program);
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
  EXPECT_EQ(pretty_log(*logging.begin()), expeted_error_message);
}

}  // namespace

