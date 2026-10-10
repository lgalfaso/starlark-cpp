// Copyright 2026 Lucas Mirelmann

#include "vm/test_assertions.hpp"

#include <format>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_types.hpp"
#include "vm/test_case.hpp"

using ::starlark::logging::logger;
using ::starlark::result::status_code;
using ::starlark::result::status_or;
using ::starlark::runtime::append_for_repr;
using ::starlark::runtime::builtin_entrypoints;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_types;
using ::starlark::runtime::to_int64_with_clamping;
using ::starlark::vm::module_loader;
using ::starlark::vm::test::make_test_loader;
using ::starlark::vm::test::split_test_case;

namespace starlark {
namespace vm {
namespace test {

starlark::vm::module_info::bindings_t make_common_test_bindings(google::protobuf::Arena& arena) {
  starlark::vm::module_info::bindings_t custom_binding;
  custom_binding["assert_eq"] = google::protobuf::Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = assert_eq_fn}, "assert_eq");
  custom_binding["assert_ne"] = google::protobuf::Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = assert_ne_fn}, "assert_ne");
  custom_binding["assert_true"] = google::protobuf::Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = assert_true_fn}, "assert_true");
  custom_binding["assert_false"] = google::protobuf::Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = assert_false_fn}, "assert_false");
  return custom_binding;
}

starlark_obj* assert_eq_fn(starlark_obj* this_obj,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  (void)this_obj;
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

starlark_obj* assert_ne_fn(starlark_obj* this_obj,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  (void)this_obj;
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

starlark_obj* assert_true_fn(starlark_obj* this_obj,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  (void)this_obj;
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

starlark_obj* assert_false_fn(starlark_obj* this_obj,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  (void)this_obj;
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

status_or<runtime_options> parse_runtime_options(starlark_obj** error,
    starlark_obj** print,
    bool* allow_static_error,
    std::ostream& out,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  auto log2_max_bigint = ctx.options().log2_max_bigint;
  auto max_sequence_size = ctx.options().max_sequence_size;
  auto max_string_length = ctx.options().max_string_length;
  auto allow_recursion = ctx.options().allow_recursion;
  for (auto& [key, value] : named_args) {
    if (value == nullptr) {
      error_callback.add_error(std::format("invalid '{}' parameter (nullptr).", key->as_string()));
      return status_or<runtime_options>(status_code::kRuntimeError);
    }
    if (key->as_string() == "print") {
      if (!is_string_kind(value->kind())) {
        error_callback.add_error(std::format("invalid '{}' parameter ({}).", key->as_string(), value->type()));
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      if (print == nullptr) {
        error_callback.add_error(std::format("Unknown named argument '{}'.", key->as_string()));
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      *print = value;
    } else if (key->as_string() == "error_message") {
      if (!is_string_kind(value->kind())) {
        error_callback.add_error(std::format("invalid '{}' parameter ({}).", key->as_string(), value->type()));
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      if (error == nullptr) {
        error_callback.add_error(std::format("Unknown named argument '{}'.", key->as_string()));
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      *error = value;
    } else if (key->as_string() == "allow_static_error") {
      if (allow_static_error == nullptr) {
        error_callback.add_error(std::format("Unknown named argument '{}'.", key->as_string()));
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      *allow_static_error = value->truthy();
    } else if (key->as_string() == "log2_max_bigint") {
      auto r = to_int64_with_clamping(*value, error_callback);
      if (!r.ok()) {
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      log2_max_bigint = *r;
    } else if (key->as_string() == "max_sequence_size") {
      auto r = to_int64_with_clamping(*value, error_callback);
      if (!r.ok()) {
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      max_sequence_size = *r;
    } else if (key->as_string() == "max_string_length") {
      auto r = to_int64_with_clamping(*value, error_callback);
      if (!r.ok()) {
        return status_or<runtime_options>(status_code::kRuntimeError);
      }
      max_string_length = *r;
    } else if (key->as_string() == "allow_recursion") {
      allow_recursion = value->truthy();
    } else {
      error_callback.add_error(std::format("Unknown named argument '{}'.", key->as_string()));
      return status_or<runtime_options>(status_code::kRuntimeError);
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

starlark_obj* check_assert_fail_with_loader(module_loader& loader,
    std::string_view source,
    const runtime_options& r_options,
    starlark_obj* expected_error,
    bool allow_static_error,
    const program_runner& run,
    logger& logging,
    context& ctx,
    error_fn& error_callback) {
  auto result = run(loader, logging, r_options);
  if (result.ok()) {
    error_callback.add_error(std::format("Program executed without errors, it was expected that it would fail.\n{}", source));
    return nullptr;
  }
  if (!allow_static_error && result.error() != status_code::kRuntimeError) {
    error_callback.add_error("Expected a runtime error, but some other found some other error.");
    for (const auto& error : logging) {
      error_callback.add_error(error.message());
    }
    return nullptr;
  }
  if (expected_error != nullptr) {
    if (logging.empty()) {
      error_callback.add_error("Expected error message but none was found.");
      return nullptr;
    }
    if (logging.begin()->message() != expected_error->str()) {
      error_callback.add_error(std::format("Actual: {}\nExpected: {}\n", logging.begin()->message(), expected_error->str()));
      return nullptr;
    }
  }
  return ctx.none_value();
}

starlark_obj* check_assert_fail(std::string_view source,
    const runtime_options& r_options,
    starlark_obj* expected_error,
    bool allow_static_error,
    const program_runner& run,
    logger& logging,
    context& ctx,
    error_fn& error_callback) {
  auto modules_parts = split_test_case(source);
  auto loader = make_test_loader(modules_parts, {});
  return check_assert_fail_with_loader(loader, source, r_options, expected_error, allow_static_error, run, logging, ctx, error_callback);
}

starlark_obj* check_assert_succeed_with_loader(module_loader& loader,
    std::string_view source,
    const runtime_options& r_options,
    starlark_obj* expected_print,
    const program_runner& run,
    logger& logging,
    std::ostringstream& out,
    context& ctx,
    error_fn& error_callback) {
  auto result = run(loader, logging, r_options);
  if (!result.ok()) {
    error_callback.add_error("Program executed with errors, it was expected that it would succeed.");
    for (const auto& entry : logging) {
      std::cerr << entry.message() << "\n";
    }
    return nullptr;
  }
  if (expected_print != nullptr) {
    std::string actual_out;
    append_for_repr(actual_out, out.str());
    if (expected_print->str() != out.str()) {
      error_callback.add_error(std::format("Output does not match. Expected {}, actual \"{}\"", expected_print->repr(), actual_out));
      return nullptr;
    }
  }
  return ctx.none_value();
}

starlark_obj* check_assert_succeed(std::string_view source,
    const runtime_options& r_options,
    starlark_obj* expected_print,
    const program_runner& run,
    logger& logging,
    std::ostringstream& out,
    context& ctx,
    error_fn& error_callback) {
  auto modules_parts = split_test_case(source);
  auto loader = make_test_loader(modules_parts, {});
  return check_assert_succeed_with_loader(loader, source, r_options, expected_print, run, logging, out, ctx, error_callback);
}

}  // namespace test
}  // namespace vm
}  // namespace starlark
