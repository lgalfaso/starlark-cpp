// Copyright 2026 Lucas Mirelmann

#include "native/exec/exec_error.hpp"

#include <cassert>

#include "native/exec/native_exec_context.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "proto/starlark_logging.pb.h"
#include "runtime/error_fn.hpp"

using ::starlark::runtime::error_fn;

namespace starlark {
namespace native {

void mark_exec_failed(native_exec_context* exec) {
  if (exec != nullptr) {
    exec->failed = true;
  }
}

void mark_module_failed(module_runtime_state* state) {
  if (state == nullptr || state->active_exec_stack.empty()) {
    return;
  }
  mark_exec_failed(state->active_exec_stack.back());
}

void report_exec_error(native_exec_context* exec, error_fn* err, std::string_view msg) {
  mark_exec_failed(exec);
  effective_err(exec, err)->add_error(msg);
}

void fail_exec_invariant(native_exec_context* exec, const char* message) {
  assert(false && message);
  mark_exec_failed(exec);
}

void fail_exec_on_runtime_error(native_exec_context* exec, error_fn* err) {
  (void)err;
  mark_exec_failed(exec);
}

void fail_exec_if_reported_error(native_exec_context* exec, bool reported_error, error_fn* err) {
  if (reported_error) {
    fail_exec_on_runtime_error(exec, err);
  }
}

exec_error_bridge::exec_error_bridge(native_exec_context* exec, error_fn* inner) : exec_(exec), inner_(inner) {}

void exec_error_bridge::add_error(std::string_view error_msg) {
  mark_exec_failed(exec_);
  inner_->add_error(error_msg);
}

void exec_error_bridge::add_error(std::string_view error_msg, std::string_view hint) {
  mark_exec_failed(exec_);
  inner_->add_error(error_msg, hint);
}

void exec_error_bridge::add_error(std::string_view error_msg, const starlark::logging::Position& pos) {
  mark_exec_failed(exec_);
  inner_->add_error(error_msg, pos);
}

void exec_error_bridge::add_error(std::string_view error_msg, const starlark::logging::Position& pos, std::string_view hint) {
  mark_exec_failed(exec_);
  inner_->add_error(error_msg, pos, hint);
}

tracked_error_bridge::tracked_error_bridge(error_fn& inner) : inner_(inner) {}

void tracked_error_bridge::add_error(std::string_view error_msg) {
  has_error_ = true;
  inner_.add_error(error_msg);
}

void tracked_error_bridge::add_error(std::string_view error_msg, std::string_view hint) {
  has_error_ = true;
  inner_.add_error(error_msg, hint);
}

void tracked_error_bridge::add_error(std::string_view error_msg, const starlark::logging::Position& pos) {
  has_error_ = true;
  inner_.add_error(error_msg, pos);
}

void tracked_error_bridge::add_error(std::string_view error_msg, const starlark::logging::Position& pos, std::string_view hint) {
  has_error_ = true;
  inner_.add_error(error_msg, pos, hint);
}

}  // namespace native
}  // namespace starlark
