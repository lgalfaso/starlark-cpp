// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_EXEC_EXEC_ERROR_HPP_
#define NATIVE_EXEC_EXEC_ERROR_HPP_

#include <string_view>

#include "proto/starlark_logging.pb.h"
#include "runtime/error_fn.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {
class error_fn;
}
namespace native {

struct module_runtime_state;
struct native_exec_context;

void mark_exec_failed(native_exec_context* exec);
void mark_module_failed(module_runtime_state* state);
void report_exec_error(native_exec_context* exec, starlark::runtime::error_fn* err, std::string_view msg);

// Internal bytecode/runtime invariant violated; not reachable from valid Starlark.
void fail_exec_invariant(native_exec_context* exec, const char* message);

// Runtime operation failed (e.g. returned nullptr); ensure a user-visible error exists.
void fail_exec_on_runtime_error(native_exec_context* exec, starlark::runtime::error_fn* err);

void fail_exec_if_reported_error(native_exec_context* exec, bool reported_error, starlark::runtime::error_fn* err);

// Marks exec failed on every reported runtime error so JIT bytecode can exit via emit_branch_if_failed.
class exec_error_bridge : public starlark::runtime::error_fn {
 public:
  exec_error_bridge(native_exec_context* exec, starlark::runtime::error_fn* inner);

  void add_error(std::string_view error_msg) override;
  void add_error(std::string_view error_msg, std::string_view hint) override;
  void add_error(std::string_view error_msg, const starlark::logging::Position& pos) override;
  void add_error(std::string_view error_msg, const starlark::logging::Position& pos, std::string_view hint) override;

 private:
  native_exec_context* exec_;
  starlark::runtime::error_fn* inner_;
};

// Tracks whether void-returning object methods reported an error before fail_exec_if_reported_error.
class tracked_error_bridge : public starlark::runtime::error_fn {
 public:
  explicit tracked_error_bridge(starlark::runtime::error_fn& inner);

  bool has_error() const { return has_error_; }

  void add_error(std::string_view error_msg) override;
  void add_error(std::string_view error_msg, std::string_view hint) override;
  void add_error(std::string_view error_msg, const starlark::logging::Position& pos) override;
  void add_error(std::string_view error_msg, const starlark::logging::Position& pos, std::string_view hint) override;

 private:
  starlark::runtime::error_fn& inner_;
  bool has_error_ = false;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_EXEC_EXEC_ERROR_HPP_
