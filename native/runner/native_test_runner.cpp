// Copyright 2026 Lucas Mirelmann

#include "native/runner/native_test_runner.hpp"

#include <unistd.h>

#include <filesystem>
#include <format>
#include <map>
#include <sstream>

#include "google/protobuf/arena.h"
#include "grammar/options.hpp"
#include "native/runner/native_options.hpp"
#include "native/runner/native_runner.hpp"
#include "native/runner/runner_state.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_types.hpp"
#include "vm/test_assertions.hpp"
#include "vm/test_case.hpp"

using ::google::protobuf::Arena;
using ::starlark::grammar::grammar_options;
using ::starlark::logging::logger;
using ::starlark::native::native_options;
using ::starlark::native::native_runner;
using ::starlark::native::native_runner_state;
using ::starlark::native::native_runtime;
using ::starlark::result::status_or;
using ::starlark::runtime::builtin_entrypoints;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_types;
using ::starlark::vm::frame;
using ::starlark::vm::module_loader;
using ::starlark::vm::test::check_assert_fail;
using ::starlark::vm::test::check_assert_succeed;
using ::starlark::vm::test::make_common_test_bindings;
using ::starlark::vm::test::make_test_loader;
using ::starlark::vm::test::parse_runtime_options;
using ::starlark::vm::test::program_runner;

namespace starlark {
namespace native_test_runner {

namespace {

std::filesystem::path native_test_cache_root() {
  return std::filesystem::temp_directory_path() / std::format("starlark-native-test-{}", ::getpid());
}

native_runner_state* as_native_runner_state(context& ctx, error_fn& error_callback) {
  auto* state = static_cast<native_runner_state*>(ctx.runner_context());
  if (state == nullptr || state->runtime == nullptr || state->loader == nullptr || state->options == nullptr) {
    error_callback.add_error("internal error: native runner state missing.");
    return nullptr;
  }
  return state;
}

program_runner make_native_program_runner(native_runner_state& state) {
  return [&state](module_loader& loader, logger& log, const runtime_options& opts) -> status_or<frame*> {
    native_runner nested(*state.runtime);
    return nested.run(loader, "main", grammar_options{}, opts, *state.options, log);
  };
}

starlark_obj* native_assert_fail_fn(starlark_obj* this_obj,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  (void)this_obj;
  auto* state = as_native_runner_state(ctx, error_callback);
  if (state == nullptr) {
    return nullptr;
  }
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
  auto* result = check_assert_fail(source_code->str(), *r_options, error, allow_static_error, make_native_program_runner(*state), logging, ctx, error_callback);
  const uint64_t keep_builtin = state->has_builtin_module_cache_key ? state->builtin_module_cache_key : 0;
  const uint64_t keep_main = state->has_main_module_cache_key ? state->main_module_cache_key : 0;
  state->runtime->discard_nested_module_states(keep_main, keep_builtin);
  state->runtime->rebind_module_loaders(*state->loader);
  state->runtime->reset_transient_state();
  return result;
}

starlark_obj* native_assert_succeed_fn(starlark_obj* this_obj,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  (void)this_obj;
  auto* state = as_native_runner_state(ctx, error_callback);
  if (state == nullptr) {
    return nullptr;
  }
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
  auto* result = check_assert_succeed(source_code->str(), *r_options, print, make_native_program_runner(*state), logging, out, ctx, error_callback);
  const uint64_t keep_builtin = state->has_builtin_module_cache_key ? state->builtin_module_cache_key : 0;
  const uint64_t keep_main = state->has_main_module_cache_key ? state->main_module_cache_key : 0;
  state->runtime->discard_nested_module_states(keep_main, keep_builtin);
  state->runtime->rebind_module_loaders(*state->loader);
  state->runtime->reset_transient_state();
  return result;
}

starlark::vm::module_info::bindings_t make_native_test_bindings(Arena& arena) {
  auto bindings = make_common_test_bindings(arena);
  bindings["assert_fail"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = native_assert_fail_fn}, "assert_fail");
  bindings["assert_succeed"] = Arena::Create<starlark_built_in_function>(&arena, nullptr, builtin_entrypoints{.call = native_assert_succeed_fn}, "assert_succeed");
  return bindings;
}

}  // namespace

status_or<frame*> run_native_test(std::map<std::string, std::string> programs, logger& logging) {
  native_runtime runtime;
  native_options n_options{.cache_root = native_test_cache_root().string()};

  Arena arena;
  auto custom_binding = make_native_test_bindings(arena);
  auto loader = make_test_loader(programs, custom_binding);

  native_runner runner(runtime);
  runtime_options options{};
  return runner.run(loader, "main", grammar_options{}, options, n_options, logging);
}

}  // namespace native_test_runner
}  // namespace starlark
