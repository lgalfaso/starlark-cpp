// Copyright 2025-2026 Lucas Mirelmann

#include "interpreter/interpreter_test_runner.hpp"

#include <format>
#include <map>
#include <sstream>
#include <string>

#include "google/protobuf/arena.h"
#include "grammar/options.hpp"
#include "interpreter/interpreter.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_types.hpp"
#include "vm/test_assertions.hpp"
#include "vm/test_case.hpp"

using ::google::protobuf::Arena;
using ::starlark::grammar::grammar_options;
using ::starlark::logging::logger;
using ::starlark::result::status_or;
using ::starlark::runtime::builtin_entrypoints;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_types;
using ::starlark::vm::frame;
using ::starlark::vm::module_info;
using ::starlark::vm::module_loader;
using ::starlark::vm::test::check_assert_fail;
using ::starlark::vm::test::check_assert_succeed;
using ::starlark::vm::test::make_common_test_bindings;
using ::starlark::vm::test::make_test_loader;
using ::starlark::vm::test::parse_runtime_options;
using ::starlark::vm::test::program_runner;
using interpreter_engine = ::starlark::interpreter::interpreter;

namespace starlark {
namespace interpreter_runner {

namespace {

status_or<frame*> run_program(interpreter_engine& runner, module_loader& loader, logger& logging, const runtime_options& r_options) {
  return runner.run(loader, "main", grammar_options{}, r_options, logging);
}

starlark_obj* interpreter_assert_fail_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  std::basic_ostringstream<char> out;
  starlark_obj* error = nullptr;
  bool allow_static_error = false;
  auto r_options = parse_runtime_options(&error, nullptr, &allow_static_error, out, named_args, ctx, error_callback);
  if (!r_options.ok()) {
    return nullptr;
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
  if (!is_string_kind(source_code->kind())) {
    error_callback.add_error(std::format("invalid 'source_code' parameter ({}).", source_code->type()));
    return nullptr;
  }
  logger logging;
  interpreter_engine nested;
  program_runner run = [&](module_loader& loader, logger& log, const runtime_options& opts) {
    return run_program(nested, loader, log, opts);
  };
  return check_assert_fail(source_code->str(), *r_options, error, allow_static_error, run, logging, ctx, error_callback);
}

starlark_obj* interpreter_assert_succeed_fn(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  std::basic_ostringstream<char> out;
  starlark_obj* print = nullptr;
  auto r_options = parse_runtime_options(nullptr, &print, nullptr, out, named_args, ctx, error_callback);
  if (!r_options.ok()) {
    return nullptr;
  }
  if (pos_args.size() != 1) {
    error_callback.add_error("assert_succeed takes only 1 positional arguments.");
    return nullptr;
  }
  auto* source_code = pos_args[0];
  if (source_code == nullptr) {
    error_callback.add_error("invalid 'source_code' parameter (nullptr).");
    return nullptr;
  }
  if (!is_string_kind(source_code->kind())) {
    error_callback.add_error(std::format("invalid 'source_code' parameter ({}).", source_code->type()));
    return nullptr;
  }
  logger logging;
  interpreter_engine nested;
  program_runner run = [&](module_loader& loader, logger& log, const runtime_options& opts) {
    return run_program(nested, loader, log, opts);
  };
  return check_assert_succeed(source_code->str(), *r_options, print, run, logging, out, ctx, error_callback);
}

starlark::vm::module_info::bindings_t make_interpreter_test_bindings(Arena& arena) {
  auto bindings = make_common_test_bindings(arena);
  bindings["assert_fail"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = interpreter_assert_fail_fn}, "assert_fail");
  bindings["assert_succeed"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = interpreter_assert_succeed_fn}, "assert_succeed");
  return bindings;
}

}  // namespace

status_or<frame*> run_test(std::map<std::string, std::string> programs, logger& logging) {
  Arena arena;
  auto custom_binding = make_interpreter_test_bindings(arena);
  auto loader = make_test_loader(programs, custom_binding);

  interpreter_engine runner;
  runtime_options options{};
  return run_program(runner, loader, logging, options);
}

}  // namespace interpreter_runner
}  // namespace starlark
