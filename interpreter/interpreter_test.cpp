// Copyright 2025-2026 Lucas Mirelmann

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <map>
#include <string>

#include "interpreter/interpreter_test_runner.hpp"
#include "io/read_file.hpp"
#include "vm/frame.hpp"

using ::starlark::interpreter_runner::run_test;
using ::starlark::logging::logger;
using ::starlark::vm::frame;
using ::testing::SizeIs;

namespace {

std::string print_logs(logger& logging) {
  std::string result;
  for (const auto& entry : logging) {
    result += std::format("Error at {}\n{}\n", entry.pos().ShortDebugString(), entry.message());
  }
  return result;
}

TEST(Interpreter, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(2));

  auto starlark_code = starlark::io::read_file(argv[1]);
  ASSERT_TRUE(starlark_code.has_value());

  logger logging;
  std::map<std::string, std::string> programs;
  programs["main"] = *starlark_code;
  auto result = run_test(programs, logging);
  ASSERT_TRUE(result.ok()) << print_logs(logging);
}

}  // namespace

