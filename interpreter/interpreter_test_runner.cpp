// Copyright 2025-2026 Lucas Mirelmann

#include <fcntl.h>

#include <format>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "bigint/number.hpp"
#include "interpreter/interpreter.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_types.hpp"
#include "third-party/defer.hpp"

using ::google::protobuf::Arena;
using ::starlark::grammar::grammar_options;
using ::starlark::interpreter::frame;
using ::starlark::interpreter::interpreter;
using ::starlark::interpreter::kv_module_loader;
using ::starlark::logging::logger;
using ::starlark::result::status_code;
using ::starlark::result::status_or;
using ::starlark::runtime::append_for_repr;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_types;
using ::starlark::runtime::to_int64_with_clamping;
using ::starlark::testing::error_handler;

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

starlark_obj* assert_ne_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!named_args.empty()) {
    error_callback.add_error("assert_ne does not take any named arguments.");
    return nullptr;
  }
  if (pos_args.size() != 2) {
    error_callback.add_error("assert_ne takes only 2 positional arguments.");
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
  if (actual->equals(*expected)) {
    error_callback.add_error(std::format("Actual: {}\nExpected: {}\n", actual->repr(), expected->repr()));
    return nullptr;
  }
  return ctx.none_value();
}

status_or<runtime_options> parse_runtime_options(starlark_obj** error, starlark_obj** print, std::ostream& out, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  auto log2_max_bigint = ctx.options().log2_max_bigint;
  auto max_sequence_size = ctx.options().max_sequence_size;
  auto max_string_length = ctx.options().max_string_length;
  auto allow_recursion = ctx.options().allow_recursion;
  for (auto& [key, value] : named_args) {
    if (value == nullptr) {
      error_callback.add_error(std::format("invalid '{}' parameter (nullptr).", key));
      return status_or<runtime_options>(status_code::kError);
    }
    if (key == "print") {
      if (value->type() != starlark_types::string_t) {
        error_callback.add_error(std::format("invalid '{}' parameter ({}).", key, value->type()));
        return status_or<runtime_options>(status_code::kError);
      }
      if (print == nullptr) {
        error_callback.add_error(std::format("Unknown named argument '{}'.", key));
        return status_or<runtime_options>(status_code::kError);
      }
      *print = value;
    } else if (key == "error_message") {
      if (value->type() != starlark_types::string_t) {
        error_callback.add_error(std::format("invalid '{}' parameter ({}).", key, value->type()));
        return status_or<runtime_options>(status_code::kError);
      }
      if (error == nullptr) {
        error_callback.add_error(std::format("Unknown named argument '{}'.", key));
        return status_or<runtime_options>(status_code::kError);
      }
      *error = value;
    } else if (key == "log2_max_bigint") {
      auto r = to_int64_with_clamping(*value, error_callback);
      if (!r.ok()) {
        return status_or<runtime_options>(status_code::kError);
      }
      log2_max_bigint = *r;
    } else if (key == "max_sequence_size") {
      auto r = to_int64_with_clamping(*value, error_callback);
      if (!r.ok()) {
        return status_or<runtime_options>(status_code::kError);
      }
      max_sequence_size = *r;
    } else if (key == "max_string_length") {
      auto r = to_int64_with_clamping(*value, error_callback);
      if (!r.ok()) {
        return status_or<runtime_options>(status_code::kError);
      }
      max_string_length = *r;
    } else if (key == "allow_recursion") {
      allow_recursion = value->truthy();
    } else {
      error_callback.add_error(std::format("Unknown named argument '{}'.", key));
      return status_or<runtime_options>(status_code::kError);
    }
  }
  runtime_options r_options = runtime_options{
    .log2_max_bigint = log2_max_bigint,
    .max_sequence_size = max_sequence_size,
    .max_string_length = max_string_length,
    .out = out,
    .allow_recursion = allow_recursion,
  };
  return status_or<runtime_options>(r_options);
}

starlark_obj* assert_fail_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  std::basic_ostringstream<char> out;
  starlark_obj* error = nullptr;
  auto r_options = parse_runtime_options(&error, nullptr, out, named_args, ctx, error_callback);
  if (!r_options.ok()) {
    return nullptr;
  }

  static char module_name[] = "//:assert_module.star";
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

  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(module_name, source_code->str(), std::map<std::string, starlark_obj*, std::less<>>{});
  kv_module_loader loader{modules};
  frame* result = runner.run(loader, module_name, grammar_options{}, *r_options, logging);
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

starlark_obj* assert_succeed_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  std::basic_ostringstream<char> out;
  starlark_obj* print = nullptr;
  auto r_options = parse_runtime_options(nullptr, &print, out, named_args, ctx, error_callback);
  if (!r_options.ok()) {
    return nullptr;
  }

  static char module_name[] = "//:assert_module.star";
  if (pos_args.size() != 1) {
    error_callback.add_error("assert_succeed takes only 1 positional arguments.");
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

  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  modules.try_emplace(module_name, source_code->str(), std::map<std::string, starlark_obj*, std::less<>>{});
  kv_module_loader loader{modules};
  frame* result = runner.run(loader, module_name, grammar_options{}, *r_options, logging);
  if (result == nullptr) {
    error_callback.add_error("Program executed with errors, it was expected that it would succeed.");
    return nullptr;
  }
  if (print != nullptr) {
    std::string actual_out;
    append_for_repr(actual_out, out.str());
    if (print->str() != out.str()) {
      error_callback.add_error(std::format("Output does not match. Expected {}, actual \"{}\"", print->repr(), actual_out));
      return nullptr;
    }
  }
  return ctx.none_value();
}

starlark_obj* assert_true_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!named_args.empty()) {
    error_callback.add_error("assert_true does not take any named arguments.");
    return nullptr;
  }
  if (pos_args.size() != 1) {
    error_callback.add_error("assert_true takes only 1 positional argument.");
    return nullptr;
  }
  auto* actual = pos_args[0];
  if (actual == nullptr) {
    error_callback.add_error("invalid 'actual' parameter (nullptr).");
    return nullptr;
  }
  if (!actual->equals(*ctx.true_value())) {
    error_callback.add_error(std::format("Actual: {}\nExpected: {}\n", actual->repr(), ctx.true_value()->repr()));
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* assert_false_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!named_args.empty()) {
    error_callback.add_error("assert_false does not take any named arguments.");
    return nullptr;
  }
  if (pos_args.size() != 1) {
    error_callback.add_error("assert_false takes only 1 positional argument.");
    return nullptr;
  }
  auto* actual = pos_args[0];
  if (actual == nullptr) {
    error_callback.add_error("invalid 'actual' parameter (nullptr).");
    return nullptr;
  }
  if (!actual->equals(*ctx.false_value())) {
    error_callback.add_error(std::format("Actual: {}\nExpected: {}\n", actual->repr(), ctx.false_value()->repr()));
    return nullptr;
  }
  return ctx.none_value();
}

}  // namespace

namespace starlark {
namespace interpreter_runner {

frame* run_test(std::map<std::string_view, std::string_view> programs, logger& logging) {
  class interpreter runner;
  Arena arena;

  std::map<std::string, std::pair<std::string, const std::map<std::string, starlark_obj*, std::less<>>>, std::less<>> modules;
  std::map<std::string, starlark_obj*, std::less<>> custom_binding;
  custom_binding["assert_eq"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, assert_eq_fn, "assert_eq");
  custom_binding["assert_fail"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, assert_fail_fn, "assert_fail");
  custom_binding["assert_succeed"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, assert_succeed_fn, "assert_succeed");
  custom_binding["assert_true"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, assert_true_fn, "assert_true");
  custom_binding["assert_false"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, assert_false_fn, "assert_false");
  for (const auto& [k, v] : programs) {
    modules.try_emplace(std::string{k}, v, custom_binding);
  }
  kv_module_loader loader{modules};

  return runner.run(loader, "main", grammar_options{}, runtime_options{}, logging);
}

}  // namespace interpreter_runner
}  // namespace starlark

