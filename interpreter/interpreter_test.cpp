// Copyright 2025 Lucas Mirelmann

#include <fcntl.h>

#include <gmock/gmock.h>
#include <gtest/gtest-matchers.h>
#include <gtest/gtest.h>

#include <format>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "bigint/number.hpp"
#include "interpreter/interpreter.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_types.hpp"
#include "third-party/defer.hpp"

using ::google::protobuf::Arena;
using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::frame;
using ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_types;
using ::starlark::testing::error_handler;
using ::testing::SizeIs;

namespace {

std::string print_logs(logger& logging) {
  std::string result;
  for (const auto& entry : logging) {
    result += std::format("Error at {}\n{}\n", entry.pos().ShortDebugString(), entry.message());
  }
  return result;
}

starlark_obj* assert_eq_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!named_args.empty()) {
    error_callback.add_error("assert_eq does not take any named arguments.");
    return nullptr;
  }
  if (pos_args.size() != 2) {
    error_callback.add_error("assert_eq takes only 2 positional arguments.");
    return nullptr;
  }
  auto* actual = pos_args[0];
  if (actual == nullptr) {
    error_callback.add_error("invalid 'actual' parameter (nullptr).");
    return nullptr;
  }
  auto* expected = pos_args[1];
  if (expected == nullptr) {
    error_callback.add_error("invalid 'expected' parameter (nullptr).");
    return nullptr;
  }
  if (!actual->equals(*expected)) {
    error_callback.add_error(std::format("Actual: {}\nExpected: {}\n", actual->repr(), expected->repr()));
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* assert_fail_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  starlark_obj* error = nullptr;
  for (auto& [key, value] : named_args) {
    if (key == "error_message") {
      if (value == nullptr) {
        error_callback.add_error("invalid 'error_message' parameter (nullptr).");
        return nullptr;
      }
      if (value->type() != starlark_types::string_t) {
        error_callback.add_error(std::format("invalid 'error_message' parameter ({}).", value->type()));
        return nullptr;
      }
      error = value;
    } else {
      error_callback.add_error(std::format("Unknown named argument '{}'.", key));
      return nullptr;
    }
  }
  if (pos_args.size() != 1) {
    error_callback.add_error("assert_fail takes only 1 positional arguments.");
    return nullptr;
  }
  auto* source_code = pos_args[0];
  if (source_code == nullptr) {
    error_callback.add_error("invalid 'source_code' parameter (nullptr).");
    return nullptr;
  }
  if (source_code->type() != starlark_types::string_t) {
    error_callback.add_error(std::format("invalid 'source_code' parameter ({}).", source_code->type()));
    return nullptr;
  }
  interpreter runner;
  logger logging;
  Arena arena2;

  frame* result = runner.run(source_code->str(), grammar_options{}, {}, arena2, logging);
  if (result != nullptr) {
    error_callback.add_error("Program executed without errors, it was expected that it would fail.");
    return nullptr;
  }
  if (error != nullptr) {
    if (logging.empty()) {
      error_callback.add_error("Expected error message but none was found.");
      return nullptr;
    }
    if (logging.begin()->message() != error->str()) {
      error_callback.add_error(std::format("Actual: {}\nExpected: {}\n", logging.begin()->message(), error->str()));
      return nullptr;
    }
  }
  return ctx.none_value();
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

  interpreter runner;
  logger logging;
  Arena arena;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  custom_binding["assert_eq"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, assert_eq_fn, "assert_eq");
  custom_binding["assert_fail"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, assert_fail_fn, "assert_fail");

  frame* result = runner.run(starlark_code, grammar_options{}, custom_binding, arena, logging);
  ASSERT_NE(nullptr, result) << print_logs(logging);
}

}  // namespace

