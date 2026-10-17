// Copyright 2026 Lucas Mirelmann

#include "differential/engine.hpp"

#include <format>
#include <unistd.h>

#include <filesystem>  // NOLINT(build/c++17)

#include "grammar/options.hpp"
#include "interpreter/interpreter.hpp"
#include "logging/logging.hpp"
#include "native/runner/native_options.hpp"
#include "native/runner/native_runner.hpp"
#include "native/runner/native_runtime.hpp"
#include "runtime/starlark_types.hpp"
#include "vm/split_test_case.hpp"

using ::starlark::grammar::grammar_options;
using interpreter_engine = ::starlark::interpreter::interpreter;
using ::starlark::logging::logger;
using ::starlark::native::jit_codegen_opt;
using ::starlark::native::native_options;
using ::starlark::native::native_runner;
using ::starlark::native::native_runtime;
using ::starlark::result::status_code;
using ::starlark::runtime::is_function_kind;
using ::starlark::runtime::runtime_options;
using ::starlark::runtime::starlark_obj;
using ::starlark::vm::frame;
using ::starlark::vm::kv_module_loader;
using ::starlark::vm::module_info;
using ::starlark::vm::try_split_test_case;

namespace starlark {
namespace differential {

namespace {

constexpr std::string_view kBeginModule = "## Begin module";

kv_module_loader make_program_loader(const std::map<std::string, std::string>& programs, const module_info::bindings_t& custom_bindings) {
  std::map<std::string, std::pair<std::string, const module_info::bindings_t>, std::less<>> modules;
  for (const auto& [name, source] : programs) {
    modules.try_emplace(name, source, custom_bindings);
  }
  return kv_module_loader{modules};
}

std::map<std::string, std::string> snapshot_exports(const frame* export_frame) {
  std::map<std::string, std::string> exports;
  if (export_frame == nullptr || export_frame->names == nullptr) {
    return exports;
  }
  const int name_count = export_frame->names->size();
  for (int i = 0; i < name_count; ++i) {
    const std::string& name = export_frame->names->Get(i);
    if (name.starts_with("_")) {
      continue;
    }
    starlark_obj* value = i < static_cast<int>(export_frame->elements.size()) ? export_frame->elements[static_cast<std::size_t>(i)] : nullptr;
    if (value == nullptr) {
      exports[name] = "<null>";
      continue;
    }
    exports[name] = value->repr();
  }
  return exports;
}

std::filesystem::path native_cache_root() {
  return std::filesystem::temp_directory_path() / std::format("starlark-differential-fuzz-{}", ::getpid());
}

}  // namespace

std::map<std::string, std::string> programs_from_input(std::string_view source) {
  if (source.starts_with(kBeginModule)) {
    auto parsed = try_split_test_case(source);
    if (parsed.has_value()) {
      return *parsed;
    }
  }
  return {{"main", std::string(source)}};
}

engine_run_snapshot run_interpreter(const std::map<std::string, std::string>& programs,
    const runtime_options& options,
    const module_info::bindings_t& custom_bindings) {
  engine_run_snapshot snapshot;
  auto loader = make_program_loader(programs, custom_bindings);
  logger logging;
  interpreter_engine runner;
  auto result = runner.run(loader, "main", grammar_options{}, options, logging);
  if (!result.ok()) {
    snapshot.category = result.error();
    return snapshot;
  }
  snapshot.category = status_code::kOk;
  snapshot.exports = snapshot_exports(*result);
  return snapshot;
}

engine_run_snapshot run_native(const std::map<std::string, std::string>& programs,
    const runtime_options& options,
    const module_info::bindings_t& custom_bindings,
    const native_options& native_run_options) {
  engine_run_snapshot snapshot;
  const jit_codegen_opt codegen =
      native_run_options.fast_jit ? jit_codegen_opt::kDefault : jit_codegen_opt::kAggressive;
  native_runtime runtime(codegen);
  native_options n_options = native_run_options;
  if (n_options.cache_root.empty()) {
    n_options.cache_root = native_cache_root().string();
  }
  auto loader = make_program_loader(programs, custom_bindings);
  logger logging;
  native_runner runner(runtime);
  auto result = runner.run(loader, "main", grammar_options{}, options, n_options, logging);
  if (!result.ok()) {
    snapshot.category = result.error();
    return snapshot;
  }
  snapshot.category = status_code::kOk;
  snapshot.exports = snapshot_exports(*result);
  return snapshot;
}

compare_result compare_snapshots(const engine_run_snapshot& interpreter, const engine_run_snapshot& native) {
  compare_result result;
  if (interpreter.category != native.category) {
    result.match = false;
    result.reason = std::format("status mismatch: interpreter={} native={}",
        static_cast<int>(interpreter.category),
        static_cast<int>(native.category));
    return result;
  }
  if (interpreter.category != status_code::kOk) {
    return result;
  }
  if (interpreter.exports != native.exports) {
    result.match = false;
    result.reason = "export mismatch";
    for (const auto& [name, interp_repr] : interpreter.exports) {
      auto it = native.exports.find(name);
      if (it == native.exports.end()) {
        result.reason += std::format("; missing in native: {}", name);
      } else if (it->second != interp_repr) {
        result.reason += std::format("; {} interpreter={} native={}", name, interp_repr, it->second);
      }
    }
    for (const auto& [name, native_repr] : native.exports) {
      if (!interpreter.exports.contains(name)) {
        result.reason += std::format("; missing in interpreter: {}", name);
      }
    }
  }
  return result;
}

compare_result check_programs(const std::map<std::string, std::string>& programs,
    const runtime_options& options,
    const module_info::bindings_t& custom_bindings,
    const native_options& native_run_options) {
  auto interpreter = run_interpreter(programs, options, custom_bindings);
  auto native = run_native(programs, options, custom_bindings, native_run_options);
  return compare_snapshots(interpreter, native);
}

compare_result check_source(std::string_view source,
    const runtime_options& options,
    const module_info::bindings_t& custom_bindings,
    const native_options& native_run_options) {
  return check_programs(programs_from_input(source), options, custom_bindings, native_run_options);
}

}  // namespace differential
}  // namespace starlark
