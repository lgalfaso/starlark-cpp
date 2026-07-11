// Copyright 2025-2026 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <map>
#include <string>

#include "interpreter/frame.hpp"
#include "interpreter/interpreter_test_runner.hpp"
#include "third-party/defer.hpp"

using ::starlark::logging::logger;
using ::starlark::interpreter::frame;
using ::starlark::interpreter_runner::run_test;
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

  std::string starlark_code;
  {
    int starlark_fd = open(argv[1].c_str(), O_RDONLY);
    ASSERT_GT(starlark_fd, 0);
    defer { close(starlark_fd); };
    struct stat sb;
    ASSERT_GE(fstat(starlark_fd, &sb), 0);
    starlark_code.resize(sb.st_size);
    read(starlark_fd, starlark_code.data(), sb.st_size);
  }

  logger logging;
  std::map<std::string, std::string> programs;
  programs["main"] = starlark_code;
  auto result = run_test(programs, logging);
  ASSERT_TRUE(result.ok()) << print_logs(logging);
}

}  // namespace

