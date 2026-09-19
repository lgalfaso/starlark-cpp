// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_NATIVE_RUNNER_HPP_
#define NATIVE_NATIVE_RUNNER_HPP_

#include <string>
#include <string_view>

#include "grammar/options.hpp"
#include "logging/logging.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "native/runner/native_options.hpp"
#include "native/runner/native_runtime.hpp"
#include "native/runner/runner_state.hpp"
#include "runtime/options.hpp"
#include "status_or/status.hpp"
#include "vm/frame.hpp"
#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

class native_runner {
 public:
  explicit native_runner(native_runtime& runtime) : runtime_(runtime) {}

  starlark::result::status_or<starlark::vm::frame*> run(starlark::vm::module_loader& loader,
      std::string_view module_name,
      const starlark::grammar::grammar_options& g_options,
      const starlark::runtime::runtime_options& r_options,
      const native_options& n_options,
      starlark::logging::logger& logging);

  // Re-run module init for an already-JIT-compiled module (steady-state benchmarks).
  starlark::result::status_or<starlark::vm::frame*> rerun(starlark::vm::module_loader& loader,
      std::string_view module_name,
      const starlark::grammar::grammar_options& g_options,
      const starlark::runtime::runtime_options& r_options,
      const native_options& n_options,
      starlark::logging::logger& logging);

 private:
  native_runtime& runtime_;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_NATIVE_RUNNER_HPP_
