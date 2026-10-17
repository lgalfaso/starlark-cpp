// Copyright 2026 Lucas Mirelmann

#ifndef DIFFERENTIAL_ENGINE_HPP_
#define DIFFERENTIAL_ENGINE_HPP_

#include <map>
#include <string>
#include <string_view>

#include "native/runner/native_options.hpp"
#include "runtime/options.hpp"
#include "status_or/status.hpp"
#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace differential {

struct engine_run_snapshot {
  starlark::result::status_code category = starlark::result::status_code::kOk;
  std::map<std::string, std::string> exports;
};

struct compare_result {
  bool match = true;
  std::string reason;
};

std::map<std::string, std::string> programs_from_input(std::string_view source);

engine_run_snapshot run_interpreter(const std::map<std::string, std::string>& programs,
    const starlark::runtime::runtime_options& options,
    const starlark::vm::module_info::bindings_t& custom_bindings);

engine_run_snapshot run_native(const std::map<std::string, std::string>& programs,
    const starlark::runtime::runtime_options& options,
    const starlark::vm::module_info::bindings_t& custom_bindings,
    const starlark::native::native_options& native_options = {});

compare_result compare_snapshots(const engine_run_snapshot& interpreter, const engine_run_snapshot& native);

compare_result check_programs(const std::map<std::string, std::string>& programs,
    const starlark::runtime::runtime_options& options,
    const starlark::vm::module_info::bindings_t& custom_bindings,
    const starlark::native::native_options& native_options = {});

compare_result check_source(std::string_view source,
    const starlark::runtime::runtime_options& options,
    const starlark::vm::module_info::bindings_t& custom_bindings,
    const starlark::native::native_options& native_options = {});

}  // namespace differential
}  // namespace starlark

#pragma GCC visibility pop

#endif  // DIFFERENTIAL_ENGINE_HPP_
