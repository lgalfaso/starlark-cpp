// Copyright 2026 Lucas Mirelmann

#include <iostream>
#include <map>
#include <string>

#include "io/read_file.hpp"
#include "native/runner/native_test_runner.hpp"
#include "vm/frame.hpp"
#include "vm/test_case.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

using ::starlark::logging::logger;
using ::starlark::native_test_runner::run_native_test;
using ::starlark::vm::frame;
using ::starlark::vm::test::split_test_case;
using ::testing::SizeIs;

namespace {

std::string print_logs(logger& logging) {
  std::string result;
  for (const auto& entry : logging) {
    result += std::format("Error at {}\n{}\n", entry.pos().ShortDebugString(), entry.message());
  }
  return result;
}

TEST(Native, TestCase) {
  const auto& argv = ::testing::internal::GetArgvs();
  ASSERT_THAT(argv, SizeIs(2));

  auto starlark_code = starlark::io::read_file(argv[1]);
  ASSERT_TRUE(starlark_code.has_value());

  logger logging;
  auto result = run_native_test(split_test_case(*starlark_code), logging);
  ASSERT_TRUE(result.ok()) << print_logs(logging);
}

}  // namespace
