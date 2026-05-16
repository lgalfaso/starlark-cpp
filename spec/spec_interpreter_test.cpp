// Copyright 2025-2026 Lucas Mirelmann

#include <fcntl.h>

#include <iostream>

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

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

std::map<std::string_view, std::string_view> split_test_case(std::string_view source) {
  std::string begin_module = "## Begin module";
  std::string end_module = "## End module";

  std::map<std::string_view, std::string_view> result;
  std::size_t start = 0;
  for (auto it = source.find(begin_module, start); it != std::string_view::npos; it = source.find(begin_module, start)) {
    auto begin_quote = source.find_first_of("\"'", it);
    if (begin_quote == std::string_view::npos) {
      std::cerr << "Invalid module\n";
      exit(1);
    }
    auto end_quote = source.find(source[begin_quote], begin_quote + 1);
    if (end_quote == std::string_view::npos) {
      std::cerr << "Invalid module name\n";
      exit(1);
    }
    auto it_end = source.find(end_module, end_quote);
    if (it_end == std::string_view::npos) {
      std::cerr << "Invalid module end\n";
      exit(1);
    }
    result[source.substr(begin_quote + 1, end_quote - begin_quote - 1)] = source.substr(it, it_end - it);
    start = it_end + end_module.size();
  }
  result["main"] = source.substr(start);
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
  frame* result = run_test(split_test_case(starlark_code), logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
}

}  // namespace

