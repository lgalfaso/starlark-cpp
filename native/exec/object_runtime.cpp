// Copyright 2026 Lucas Mirelmann

#include "native/exec/object_runtime.hpp"

#include <cassert>

#include <algorithm>
#include <limits>
#include <optional>
#include <span>
#include <string>

#include "bigint/number.hpp"
#include "errors/runtime_error_messages.hpp"
#include "google/protobuf/arena.h"
#include "native/abi/starlark_module_abi.hpp"
#include "native/exec/exec_error.hpp"
#include "native/exec/native_exec_context.hpp"
#include "native/exec/native_function.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "proto/starlark_logging.pb.h"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/object_kind.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_numeric.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"
#include "vm/fast_compare.hpp"
#include "vm/frame.hpp"
#include "vm/frame_factory.hpp"
#include "vm/module_metadata.hpp"

namespace starlark {
namespace native {

namespace {

using ::starlark::runtime::context;
using ::starlark::runtime::create_integer;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::is_dict_kind;
using ::starlark::runtime::is_function_kind;
using ::starlark::runtime::is_string_kind;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;
using ::starlark::vm::function_metadata_for_block;

void ensure_eval_stack_capacity_for(native_exec_context* exec, uint32_t required_capacity) {
  if (exec == nullptr || exec->mod == nullptr) {
    return;
  }
  if (exec->eval_stack.capacity >= required_capacity) {
    return;
  }
  auto& buffer = exec->mod->stack_buffer;
  uint32_t new_cap = std::max(required_capacity, exec->eval_stack.capacity * 2 + 64);
  buffer.resize(new_cap);
  exec->eval_stack.data = buffer.data();
  exec->eval_stack.capacity = static_cast<uint32_t>(buffer.size());
}

void eval_stack_push(native_exec_context* exec, starlark_obj* value) {
  ensure_eval_stack_capacity_for(exec, exec->eval_stack.size + 1);
  exec->eval_stack.push(value);
}

void binary_op(native_exec_context* exec, context* ctx, error_fn* err, auto method) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  if (exec->failed || exec->eval_stack.size < 2) {
    return;
  }
  auto* rhs = exec->eval_stack.pop();
  auto* lhs = exec->eval_stack.peek();
  exec->eval_stack.data[exec->eval_stack.size - 1] = (lhs->*method)(*rhs, *ctx, *err);
}

void unary_op(native_exec_context* exec, context* ctx, error_fn* err, auto method) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  if (exec->failed || exec->eval_stack.size == 0) {
    return;
  }
  exec->eval_stack.data[exec->eval_stack.size - 1] = (exec->eval_stack.peek()->*method)(*ctx, *err);
}

starlark_obj* invoke_call_pos(native_exec_context* exec, starlark_obj* callee, std::span<starlark_obj*> args, context* ctx, error_fn* err) {
  if (callee == nullptr) {
    fail_exec_invariant(exec, "eval stack callee must not be null");
    return nullptr;
  }
  auto* result = callee->call_pos(args, *ctx, *err);
  if (result == nullptr) {
    fail_exec_on_runtime_error(exec, err);
  }
  return result;
}

void execute_call_pos(jit_value_stack& stack, std::size_t arg_count, native_exec_context& exec, context& ctx, error_fn& error_callback) {
  active_exec_stack_guard exec_guard(exec.mod, &exec);

  const uint32_t callee_idx = stack.size - static_cast<uint32_t>(arg_count) - 1;
  starlark_obj* callee = stack.at(callee_idx);
  if (callee == nullptr) {
    fail_exec_invariant(&exec, "eval stack callee must not be null");
    stack.resize(callee_idx + 1);
    stack.data[stack.size - 1] = nullptr;
    return;
  }
  starlark_obj* result = callee->call_pos(std::span<starlark_obj*>(stack.data + callee_idx + 1, arg_count), ctx, error_callback);
  if (result == nullptr) {
    fail_exec_on_runtime_error(&exec, &error_callback);
  }
  stack.resize(callee_idx + 1);
  stack.data[stack.size - 1] = result;
}

void execute_call_method_pos(jit_value_stack& stack, std::string_view member, std::size_t arg_count, native_exec_context& exec, context& ctx, error_fn& error_callback) {
  const uint32_t receiver_idx = stack.size - static_cast<uint32_t>(arg_count) - 1;
  starlark_obj* receiver = stack.at(receiver_idx);
  if (receiver == nullptr) {
    fail_exec_invariant(&exec, "call_method eval stack receiver must not be null");
    stack.resize(receiver_idx + 1);
    stack.data[stack.size - 1] = nullptr;
    return;
  }
  starlark_obj* result = receiver->call_method(member, std::span<starlark_obj*>(stack.data + receiver_idx + 1, arg_count), ctx, error_callback);
  if (result == nullptr) {
    fail_exec_on_runtime_error(&exec, &error_callback);
  }
  stack.resize(receiver_idx + 1);
  stack.data[stack.size - 1] = result;
}

starlark_obj* compound_method(starlark_obj* value, int32_t op_kind, starlark_obj* rhs, context& ctx, error_fn& err) {
  switch (op_kind) {
    case 0:
      return value->plus_equals_assign(*rhs, ctx, err);
    case 1:
      return value->minus_equals_assign(*rhs, ctx, err);
    case 2:
      return value->star_equals_assign(*rhs, ctx, err);
    case 3:
      return value->slash_equals_assign(*rhs, ctx, err);
    case 4:
      return value->slash_slash_equals_assign(*rhs, ctx, err);
    case 5:
      return value->percent_equals_assign(*rhs, ctx, err);
    case 6:
      return value->ampersand_equals_assign(*rhs, ctx, err);
    case 7:
      return value->pipe_equals_assign(*rhs, ctx, err);
    case 8:
      return value->hat_equals_assign(*rhs, ctx, err);
    case 9:
      return value->less_less_equals_assign(*rhs, ctx, err);
    case 10:
      return value->greater_greater_equals_assign(*rhs, ctx, err);
    default:
      return nullptr;
  }
}

void slice_range_assign_op(starlark::vm::frame* container_frame,
    starlark_obj* container,
    starlark_obj* start,
    starlark_obj* stop,
    starlark_obj* stride,
    starlark_obj* rhs,
    int32_t op_kind,
    context& ctx,
    error_fn& err) {
  switch (op_kind) {
    case 0:
      container->slice_range_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 1:
      container->slice_range_plus_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 2:
      container->slice_range_minus_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 3:
      container->slice_range_star_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 4:
      container->slice_range_slash_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 5:
      container->slice_range_slash_slash_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 6:
      container->slice_range_percent_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 7:
      container->slice_range_ampersand_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 8:
      container->slice_range_pipe_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 9:
      container->slice_range_hat_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 10:
      container->slice_range_less_less_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    case 11:
      container->slice_range_greater_greater_equals_assign(*start, *stop, *stride, *rhs, ctx, err);
      break;
    default:
      break;
  }
  (void)container_frame;
}

void pin_live_closure_frame(starlark::vm::frame* frame, native_exec_context* exec) {
  if (frame == nullptr || exec == nullptr || exec->mod == nullptr || exec->mod->fns_in_stack.empty()) {
    return;
  }
  if (exec->frame_chain.depth == 0 || frame != exec->frame_chain.back()) {
    return;
  }
  frame->pinned_for_closure = true;
}

}  // namespace

extern "C" {

starlark_obj* starlark_obj_rt_create_int(context* ctx, int64_t value) {
  return create_integer(value, *ctx);
}

starlark_obj* starlark_obj_rt_create_float(context* ctx, double value) {
  return google::protobuf::Arena::Create<starlark_float>(&ctx->arena(), value);
}

starlark_obj* starlark_obj_rt_get_const_string(native_exec_context* exec, int64_t index) {
  module_runtime_state* state = exec == nullptr ? nullptr : (exec->code_mod != nullptr ? exec->code_mod : exec->mod);
  if (state == nullptr) {
    return nullptr;
  }
  const auto& pool = state->const_strings;
  if (index < 0 || static_cast<std::size_t>(index) >= pool.size()) {
    fail_exec_invariant(exec, "const string index out of range");
    return nullptr;
  }
  return pool[static_cast<std::size_t>(index)];
}

starlark_obj* starlark_obj_rt_load_frame_slot(native_exec_context* exec, int32_t frame, int32_t slot, error_fn* err) {
  err = effective_err(exec, err);
  if (exec == nullptr || frame < 0 || static_cast<uint32_t>(frame) >= exec->frame_chain.depth) {
    return nullptr;
  }
  auto* frm = exec->frame_chain.at(static_cast<uint32_t>(frame));
  if (slot < 0 || static_cast<std::size_t>(slot) >= frm->elements.size()) {
    return nullptr;
  }
  auto* value = frm->elements[static_cast<std::size_t>(slot)];
  if (value == nullptr && frm->names != nullptr && slot < frm->names->size()) {
    report_exec_error(exec, err, starlark::error_messages::error_v2_unbound_variable(frm->names->Get(slot)));
  }
  return value;
}

void starlark_obj_rt_ensure_stack_capacity(native_exec_context* exec, int32_t required_capacity) {
  if (required_capacity < 0) {
    return;
  }
  ensure_eval_stack_capacity_for(exec, static_cast<uint32_t>(required_capacity));
}

void starlark_obj_rt_binary_plus(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_plus);
}
void starlark_obj_rt_binary_minus(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_minus);
}
void starlark_obj_rt_binary_star(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_star);
}
void starlark_obj_rt_binary_slash(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_slash);
}
void starlark_obj_rt_binary_slash_slash(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_slash_slash);
}
void starlark_obj_rt_binary_percent(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_percent);
}
void starlark_obj_rt_binary_ampersand(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_and);
}
void starlark_obj_rt_binary_pipe(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_pipe);
}
void starlark_obj_rt_binary_hat(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_hat);
}
void starlark_obj_rt_binary_lshift(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_lshift);
}
void starlark_obj_rt_binary_rshift(native_exec_context* exec, context* ctx, error_fn* err) {
  binary_op(exec, ctx, err, &starlark_obj::binary_rshift);
}

void starlark_obj_rt_unary_not(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  (void)err;
  if (exec->eval_stack.size == 0) {
    return;
  }
  exec->eval_stack.data[exec->eval_stack.size - 1] =
      exec->eval_stack.peek()->truthy() ? ctx->false_value() : ctx->true_value();
}
void starlark_obj_rt_unary_plus(native_exec_context* exec, context* ctx, error_fn* err) {
  unary_op(exec, ctx, err, &starlark_obj::unary_plus);
}
void starlark_obj_rt_unary_minus(native_exec_context* exec, context* ctx, error_fn* err) {
  unary_op(exec, ctx, err, &starlark_obj::unary_minus);
}
void starlark_obj_rt_unary_tilde(native_exec_context* exec, context* ctx, error_fn* err) {
  unary_op(exec, ctx, err, &starlark_obj::unary_tilde);
}

void starlark_obj_rt_cmp_eq(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  (void)err;
  auto* rhs = exec->eval_stack.pop();
  exec->eval_stack.data[exec->eval_stack.size - 1] = starlark::vm::fast_equals(exec->eval_stack.peek(), rhs) ? ctx->true_value() : ctx->false_value();
}
void starlark_obj_rt_cmp_ne(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  (void)err;
  auto* rhs = exec->eval_stack.pop();
  exec->eval_stack.data[exec->eval_stack.size - 1] = starlark::vm::fast_equals(exec->eval_stack.peek(), rhs) ? ctx->false_value() : ctx->true_value();
}
void starlark_obj_rt_cmp_lt(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* rhs = exec->eval_stack.pop();
  if (auto cmp = starlark::vm::fast_int_cmp(exec->eval_stack.peek(), rhs)) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp < 0 ? ctx->true_value() : ctx->false_value());
    return;
  }
  auto cmp = exec->eval_stack.peek()->cmp(*rhs, "<", *err);
  if (cmp.ok()) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp < 0 ? ctx->true_value() : ctx->false_value());
  }
}
void starlark_obj_rt_cmp_le(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* rhs = exec->eval_stack.pop();
  if (auto cmp = starlark::vm::fast_int_cmp(exec->eval_stack.peek(), rhs)) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp <= 0 ? ctx->true_value() : ctx->false_value());
    return;
  }
  auto cmp = exec->eval_stack.peek()->cmp(*rhs, "<=", *err);
  if (cmp.ok()) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp <= 0 ? ctx->true_value() : ctx->false_value());
  }
}
void starlark_obj_rt_cmp_gt(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* rhs = exec->eval_stack.pop();
  if (auto cmp = starlark::vm::fast_int_cmp(exec->eval_stack.peek(), rhs)) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp > 0 ? ctx->true_value() : ctx->false_value());
    return;
  }
  auto cmp = exec->eval_stack.peek()->cmp(*rhs, ">", *err);
  if (cmp.ok()) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp > 0 ? ctx->true_value() : ctx->false_value());
  }
}
void starlark_obj_rt_cmp_ge(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* rhs = exec->eval_stack.pop();
  if (auto cmp = starlark::vm::fast_int_cmp(exec->eval_stack.peek(), rhs)) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp >= 0 ? ctx->true_value() : ctx->false_value());
    return;
  }
  auto cmp = exec->eval_stack.peek()->cmp(*rhs, ">=", *err);
  if (cmp.ok()) {
    exec->eval_stack.data[exec->eval_stack.size - 1] = (*cmp >= 0 ? ctx->true_value() : ctx->false_value());
  }
}
void starlark_obj_rt_cmp_in(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* sequence = exec->eval_stack.pop();
  exec->eval_stack.data[exec->eval_stack.size - 1] = sequence->binary_in(*exec->eval_stack.peek(), *err) ? ctx->true_value() : ctx->false_value();
}
void starlark_obj_rt_cmp_not_in(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* sequence = exec->eval_stack.pop();
  exec->eval_stack.data[exec->eval_stack.size - 1] = sequence->binary_in(*exec->eval_stack.peek(), *err) ? ctx->false_value() : ctx->true_value();
}

bool starlark_obj_rt_truthy(starlark_obj* value) {
  return value != nullptr && value->truthy();
}

starlark_obj* starlark_obj_rt_ctx_none_value(context* ctx) {
  return ctx->none_value();
}
starlark_obj* starlark_obj_rt_ctx_true_value(context* ctx) {
  return ctx->true_value();
}
starlark_obj* starlark_obj_rt_ctx_false_value(context* ctx) {
  return ctx->false_value();
}
int32_t starlark_obj_rt_obj_numeric_type(starlark_obj* value) {
  return value == nullptr ? starlark::runtime::kNumericAbiNotNumeric : starlark::runtime::kind_to_numeric_abi(value->kind());
}
int64_t starlark_obj_rt_obj_as_int64(starlark_obj* value) {
  return value == nullptr ? 0 : value->as_int64();
}

void starlark_obj_rt_dot(native_exec_context* exec, const char* member, std::size_t len, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  exec->eval_stack.data[exec->eval_stack.size - 1] = exec->eval_stack.peek()->dot(std::string_view{member, len}, *ctx, *err);
}
void starlark_obj_rt_index(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* index = exec->eval_stack.pop();
  exec->eval_stack.data[exec->eval_stack.size - 1] = exec->eval_stack.peek()->index(*index, *ctx, *err);
}
void starlark_obj_rt_slice_range(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* stride = exec->eval_stack.pop();
  auto* stop = exec->eval_stack.pop();
  auto* start = exec->eval_stack.pop();
  auto* element = exec->eval_stack.peek();
  exec->eval_stack.data[exec->eval_stack.size - 1] = element->slice_range(*start, *stop, *stride, *ctx, *err);
}

bool starlark_obj_rt_is_native_function(starlark_obj* value) {
  if (value == nullptr || !is_function_kind(value->kind())) {
    return false;
  }
  return value != nullptr && is_function_kind(value->kind());
}

void* starlark_obj_rt_native_fn_code_ptr(starlark_obj* value) {
  if (!starlark_obj_rt_is_native_function(value)) {
    return nullptr;
  }
  auto* native_fn = static_cast<native_starlark_function*>(value);
  void* code_ptr = native_fn->code_ptr();
  return code_ptr;
}

bool starlark_obj_rt_native_fn_direct_eligible(starlark_obj* value) {
  if (!starlark_obj_rt_is_native_function(value)) {
    return false;
  }
  return static_cast<native_starlark_function*>(value)->direct_dispatch_eligible();
}

void starlark_obj_rt_call_native_direct(native_exec_context* exec, int32_t pos_count, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  if (exec->failed || exec->eval_stack.size < static_cast<uint32_t>(pos_count) + 1) {
    return;
  }
  const uint32_t callee_idx = exec->eval_stack.size - static_cast<uint32_t>(pos_count) - 1;
  starlark_obj* callee = exec->eval_stack.at(callee_idx);
  if (!starlark_obj_rt_is_native_function(callee)) {
    fail_exec_invariant(exec, "call_native_direct requires native function callee");
    return;
  }
  auto* native_fn = static_cast<native_starlark_function*>(callee);
  module_runtime_state* mod = native_fn->mod_state();
  if (mod == nullptr || mod->program == nullptr) {
    fail_exec_invariant(exec, "native function has no module state");
    return;
  }
  const int32_t block_idx = native_fn->block_idx();
  if (block_idx <= 0 || static_cast<std::size_t>(block_idx - 1) >= mod->init_metadata.functions.size()) {
    execute_call_pos(exec->eval_stack, pos_count, *exec, *ctx, *err);
    return;
  }
  const auto& fn_meta = mod->init_metadata.functions[static_cast<std::size_t>(block_idx - 1)];
  if (!native_fn->direct_dispatch_eligible() || pos_count != fn_meta.positional_param_count) {
    execute_call_pos(exec->eval_stack, pos_count, *exec, *ctx, *err);
    return;
  }
  if (starlark_obj_rt_native_fn_code_ptr(callee) == nullptr) {
    fail_exec_invariant(exec, "native function has no code pointer");
    return;
  }
  (void)native_fn->invoke_jit(exec, nullptr, pos_count, true, ctx, err);
}

bool starlark_obj_rt_try_predeclared_call_pos(native_exec_context* exec, int32_t pos_count, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  if (exec->failed || pos_count < 0 || exec->eval_stack.size < static_cast<uint32_t>(pos_count) + 1) {
    return false;
  }
  const uint32_t callee_idx = exec->eval_stack.size - static_cast<uint32_t>(pos_count) - 1;
  starlark_obj* callee = exec->eval_stack.at(callee_idx);
  if (!is_builtin_function_kind(callee->kind())) {
    return false;
  }
  std::vector<starlark_obj*> args(static_cast<std::size_t>(pos_count));
  for (int i = 0; i < pos_count; ++i) {
    args[static_cast<std::size_t>(i)] = exec->eval_stack.at(callee_idx + 1 + static_cast<uint32_t>(i));
  }
  starlark_obj* result = callee->call_pos(args, *ctx, *err);
  if (result == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return false;
  }
  exec->eval_stack.resize(callee_idx + 1);
  exec->eval_stack.data[callee_idx] = result;
  return true;
}

void starlark_obj_rt_call_pos0(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  exec->eval_stack.data[exec->eval_stack.size - 1] = invoke_call_pos(exec, exec->eval_stack.peek(), {}, ctx, err);
}
void starlark_obj_rt_call_pos1(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* arg0 = exec->eval_stack.pop();
  starlark_obj* args[] = {arg0};
  exec->eval_stack.data[exec->eval_stack.size - 1] = invoke_call_pos(exec, exec->eval_stack.peek(), std::span<starlark_obj*>(args, 1), ctx, err);
}
void starlark_obj_rt_call_pos2(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* arg1 = exec->eval_stack.pop();
  auto* arg0 = exec->eval_stack.pop();
  starlark_obj* args[] = {arg0, arg1};
  exec->eval_stack.data[exec->eval_stack.size - 1] = invoke_call_pos(exec, exec->eval_stack.peek(), std::span<starlark_obj*>(args, 2), ctx, err);
}
void starlark_obj_rt_call_pos3(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* arg2 = exec->eval_stack.pop();
  auto* arg1 = exec->eval_stack.pop();
  auto* arg0 = exec->eval_stack.pop();
  starlark_obj* args[] = {arg0, arg1, arg2};
  exec->eval_stack.data[exec->eval_stack.size - 1] = invoke_call_pos(exec, exec->eval_stack.peek(), std::span<starlark_obj*>(args, 3), ctx, err);
}
void starlark_obj_rt_call_pos(native_exec_context* exec, int32_t count, context* ctx, error_fn* err) {
  execute_call_pos(exec->eval_stack, count, *exec, *effective_ctx(exec, ctx), *effective_err(exec, err));
}
void starlark_obj_rt_call_named(native_exec_context* exec, int32_t pos_count, int32_t named_count, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  const int args_count = pos_count + 2 * named_count;
  if (args_count < 0 || exec->eval_stack.size < static_cast<uint32_t>(args_count) + 1) {
    fail_exec_invariant(exec, "call_named eval stack underflow");
    return;
  }
  const uint32_t callee_idx = exec->eval_stack.size - static_cast<uint32_t>(args_count) - 1;
  auto* callee = exec->eval_stack.at(callee_idx);
  starlark_obj::pos_args_t pos_args;
  for (int i = 0; i < pos_count; ++i) {
    pos_args.push_back(exec->eval_stack.at(callee_idx + 1 + static_cast<uint32_t>(i)));
  }
  starlark_obj::named_args_t named_args;
  for (int i = 0; i < named_count; ++i) {
    auto* key = static_cast<starlark_string*>(exec->eval_stack.at(callee_idx + 1 + static_cast<uint32_t>(pos_count) + 2 * static_cast<uint32_t>(i)));
    auto* value = exec->eval_stack.at(callee_idx + 1 + static_cast<uint32_t>(pos_count) + 2 * static_cast<uint32_t>(i) + 1);
    named_args.emplace_back(key, value);
  }
  exec->eval_stack.resize(callee_idx + 1);
  auto* result = callee->call(pos_args, named_args, *ctx, *err);
  exec->eval_stack.data[exec->eval_stack.size - 1] = result;
  if (result == nullptr) {
    fail_exec_on_runtime_error(exec, err);
  }
}

void starlark_obj_rt_call_method_pos0(native_exec_context* exec, const char* member, std::size_t len, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  exec->eval_stack.data[exec->eval_stack.size - 1] = exec->eval_stack.peek()->call_method(std::string_view{member, len}, {}, *ctx, *err);
}
void starlark_obj_rt_call_method_pos1(native_exec_context* exec, const char* member, std::size_t len, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* arg0 = exec->eval_stack.pop();
  starlark_obj* args[] = {arg0};
  exec->eval_stack.data[exec->eval_stack.size - 1] = exec->eval_stack.peek()->call_method(std::string_view{member, len}, std::span<starlark_obj*>(args, 1), *ctx, *err);
}
void starlark_obj_rt_call_method_pos2(native_exec_context* exec, const char* member, std::size_t len, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* arg1 = exec->eval_stack.pop();
  auto* arg0 = exec->eval_stack.pop();
  starlark_obj* args[] = {arg0, arg1};
  exec->eval_stack.data[exec->eval_stack.size - 1] = exec->eval_stack.peek()->call_method(std::string_view{member, len}, std::span<starlark_obj*>(args, 2), *ctx, *err);
}
void starlark_obj_rt_call_method_pos3(native_exec_context* exec, const char* member, std::size_t len, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* arg2 = exec->eval_stack.pop();
  auto* arg1 = exec->eval_stack.pop();
  auto* arg0 = exec->eval_stack.pop();
  starlark_obj* args[] = {arg0, arg1, arg2};
  exec->eval_stack.data[exec->eval_stack.size - 1] = exec->eval_stack.peek()->call_method(std::string_view{member, len}, std::span<starlark_obj*>(args, 3), *ctx, *err);
}
void starlark_obj_rt_call_method_pos(native_exec_context* exec, const char* member, std::size_t len, int32_t count, context* ctx, error_fn* err) {
  execute_call_method_pos(exec->eval_stack, std::string_view{member, len}, count, *exec, *effective_ctx(exec, ctx), *effective_err(exec, err));
}
void starlark_obj_rt_call_pos_star(native_exec_context* exec, int32_t pos_count, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  const int args_count = pos_count + 1;
  starlark_obj::pos_args_t pos_args;
  for (int i = 0; i < pos_count; ++i) {
    pos_args.push_back(exec->eval_stack.at(exec->eval_stack.size - static_cast<uint32_t>(args_count) + static_cast<uint32_t>(i)));
  }
  auto* iterable = exec->eval_stack.at(exec->eval_stack.size - static_cast<uint32_t>(args_count) + static_cast<uint32_t>(pos_count));
  if (iterable == nullptr) {
    fail_exec_invariant(exec, "call_pos_star iterable must not be null");
    return;
  }
  auto* it = iterable->get_iterator(true, *ctx, *err);
  if (it == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return;
  }
  while (it->has_next()) {
    pos_args.push_back(it->next());
  }
  it->end_iterator();
  const uint32_t callee_idx = exec->eval_stack.size - static_cast<uint32_t>(args_count) - 1;
  starlark_obj* callee = exec->eval_stack.at(callee_idx);
  starlark_obj* result = invoke_call_pos(exec, callee, pos_args, ctx, err);
  exec->eval_stack.resize(callee_idx + 1);
  exec->eval_stack.data[exec->eval_stack.size - 1] = result;
}
void starlark_obj_rt_call_full(native_exec_context* exec, int32_t positional_count, int32_t named_count, bool has_var_pos, bool has_var_named, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  int args_count = positional_count + 2 * named_count + (has_var_pos ? 1 : 0) + (has_var_named ? 1 : 0);
  const uint32_t args_base = exec->eval_stack.size - static_cast<uint32_t>(args_count);
  const uint32_t named_args_base = args_base + static_cast<uint32_t>(positional_count);
  starlark_obj::pos_args_t call_pos_args;
  starlark_obj::named_args_t call_named_args;
  for (int i = 0; i < positional_count; ++i) {
    call_pos_args.push_back(exec->eval_stack.at(args_base + static_cast<uint32_t>(i)));
  }
  for (int i = 0; i < named_count; ++i) {
    auto* key_obj = static_cast<starlark_string*>(exec->eval_stack.at(named_args_base + 2 * static_cast<uint32_t>(i)));
    auto* value = exec->eval_stack.at(named_args_base + 2 * static_cast<uint32_t>(i) + 1);
    call_named_args.emplace_back(key_obj, value);
  }
  if (has_var_pos) {
    auto* iterable = exec->eval_stack.at(named_args_base + 2 * static_cast<uint32_t>(named_count));
    auto* it = iterable->get_iterator(true, *ctx, *err);
    if (it != nullptr) {
      while (it->has_next()) {
        call_pos_args.push_back(it->next());
      }
      it->end_iterator();
    }
  }
  if (has_var_named) {
    auto* kwargs_dict = static_cast<starlark_dictionary*>(exec->eval_stack.peek());
    if (!is_dict_kind(kwargs_dict->kind())) {
      report_exec_error(exec, err, starlark::error_messages::error_v2_expect_mapping_after_star_star(kwargs_dict->type()));
      return;
    }
    for (int i = 0; i < named_count; ++i) {
      auto* key_obj = static_cast<starlark_string*>(exec->eval_stack.at(named_args_base + 2 * static_cast<uint32_t>(i)));
      if (kwargs_dict->binary_in(*key_obj, *err)) {
        report_exec_error(exec, err, starlark::error_messages::error_v2_multiple_values_for_keyword(key_obj->as_string()));
        return;
      }
    }
    auto* it = kwargs_dict->get_iterator(true, *ctx, *err);
    if (it != nullptr) {
      while (it->has_next()) {
        auto* key = it->next();
        if (!is_string_kind(key->kind())) {
          report_exec_error(exec, err, starlark::error_messages::error_v2_keyword_must_be_string());
          return;
        }
        auto* value = kwargs_dict->index(*key, *ctx, *err);
        if (value != nullptr) {
          call_named_args.emplace_back(key, value);
        }
      }
      it->end_iterator();
    }
  }
  exec->eval_stack.resize(exec->eval_stack.size - static_cast<uint32_t>(args_count));
  auto* callee = exec->eval_stack.peek();
  auto* result = callee->call(call_pos_args, call_named_args, *ctx, *err);
  exec->eval_stack.data[exec->eval_stack.size - 1] = result;
  if (result == nullptr) {
    fail_exec_on_runtime_error(exec, err);
  }
}

void starlark_obj_rt_make_list(native_exec_context* exec, int32_t reserve, context* ctx) {
  ctx = effective_ctx(exec, ctx);
  eval_stack_push(exec, google::protobuf::Arena::Create<starlark_list>(&ctx->arena(), reserve));
}
void starlark_obj_rt_make_dict(native_exec_context* exec, context* ctx) {
  ctx = effective_ctx(exec, ctx);
  eval_stack_push(exec, google::protobuf::Arena::Create<starlark_dictionary>(&ctx->arena()));
}
void starlark_obj_rt_add_to_list(native_exec_context* exec, int32_t count, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  const uint32_t element_count = static_cast<uint32_t>(count);
  auto* list = static_cast<starlark_list*>(exec->eval_stack.at(exec->eval_stack.size - 1 - element_count));
  for (int i = 0; i < count; ++i) {
    list->append(exec->eval_stack.at(exec->eval_stack.size - element_count + static_cast<uint32_t>(i)), *ctx, *err);
  }
  exec->eval_stack.resize(exec->eval_stack.size - element_count);
}
void starlark_obj_rt_add_to_dict(native_exec_context* exec, int32_t count, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  const uint32_t element_count = static_cast<uint32_t>(count);
  auto* dict = static_cast<starlark_dictionary*>(exec->eval_stack.at(exec->eval_stack.size - 1 - 2 * element_count));
  for (int i = 0; i < count; ++i) {
    auto* key = exec->eval_stack.at(exec->eval_stack.size - 2 * element_count + 2 * static_cast<uint32_t>(i));
    if (!dict->insert(key, exec->eval_stack.at(exec->eval_stack.size - 2 * element_count + 2 * static_cast<uint32_t>(i) + 1), *err).first && count > 1) {
      report_exec_error(exec, err, starlark::error_messages::error_v2_dictionary_duplicate_key(key->repr()));
      return;
    }
  }
  exec->eval_stack.resize(exec->eval_stack.size - 2 * element_count);
}
void starlark_obj_rt_make_bigint(native_exec_context* exec, const char* digits, context* ctx) {
  ctx = effective_ctx(exec, ctx);
  eval_stack_push(exec, google::protobuf::Arena::Create<starlark_bigint>(&ctx->arena(), starlark::bigint::parse_number(digits, nullptr, 0)));
}
void starlark_obj_rt_make_bytes(native_exec_context* exec, const char* data, std::size_t len, context* ctx) {
  ctx = effective_ctx(exec, ctx);
  eval_stack_push(exec, google::protobuf::Arena::Create<starlark_bytes>(&ctx->arena(), std::string{std::string_view{data, len}}));
}

void starlark_obj_rt_unpack(native_exec_context* exec, int32_t count, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* element = exec->eval_stack.pop();
  std::vector<starlark_obj*> unpacked;
  element->unpack(count, unpacked, *ctx, *err);
  for (auto* value : unpacked) {
    eval_stack_push(exec, value);
  }
}
void starlark_obj_rt_get_iterator(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* iterable = exec->eval_stack.pop();
  if (iterable == nullptr) {
    fail_exec_invariant(exec, "for-loop iterable must not be null");
    return;
  }
  auto* it = iterable->get_iterator(true, *ctx, *err);
  if (it == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return;
  }
  exec->frame_chain.back()->iterators.push_back(it);
}
void* starlark_obj_rt_exec_frame_back_iterator(native_exec_context* exec) {
  if (exec == nullptr || exec->frame_chain.depth == 0) {
    return nullptr;
  }
  auto* frame = exec->frame_chain.back();
  if (frame == nullptr || frame->iterators.empty()) {
    return nullptr;
  }
  return frame->iterators.back();
}
bool starlark_obj_rt_iterator_has_next(void* iterator) {
  auto* it = static_cast<starlark::runtime::starlark_iterator*>(iterator);
  return it != nullptr && it->has_next();
}
bool starlark_obj_rt_iterator_next_push(native_exec_context* exec, void* iterator) {
  auto* it = static_cast<starlark::runtime::starlark_iterator*>(iterator);
  if (it == nullptr) {
    mark_exec_failed(exec);
    return false;
  }
  if (it->has_next()) {
    eval_stack_push(exec, it->next());
    return true;
  }
  return false;
}
bool starlark_obj_rt_iterator_next_ext(void* iterator) {
  auto* it = static_cast<starlark::runtime::starlark_iterator*>(iterator);
  if (it == nullptr) {
    return false;
  }
  if (it->has_next()) {
    it->next_ext();
    return true;
  }
  return false;
}
void starlark_obj_rt_end_iterator(native_exec_context* exec) {
  auto* it = exec->frame_chain.back()->iterators.back();
  if (it != nullptr) {
    it->end_iterator();
  }
  exec->frame_chain.back()->iterators.pop_back();
}

void starlark_obj_rt_make_tuple(native_exec_context* exec, int32_t count, context* ctx) {
  ctx = effective_ctx(exec, ctx);
  auto* result = google::protobuf::Arena::Create<starlark_tuple>(&ctx->arena(), count);
  for (int i = count; i > 0; --i) {
    result->add(exec->eval_stack.at(exec->eval_stack.size - static_cast<uint32_t>(i)));
  }
  exec->eval_stack.resize(exec->eval_stack.size - static_cast<uint32_t>(count));
  eval_stack_push(exec, result);
}

void starlark_obj_rt_make_native_function_meta(native_exec_context* exec,
    void* fn_ptr,
    const char* fn_name,
    std::size_t fn_name_len,
    int32_t block_idx,
    int32_t default_count) {
  auto* ctx = effective_ctx(exec, nullptr);
  if (fn_ptr == nullptr) {
    fail_exec_invariant(exec, "native function pointer must be available at make_function");
    return;
  }
  std::vector<starlark_obj*> defaults;
  defaults.reserve(default_count);
  for (int i = default_count; i > 0; --i) {
    defaults.push_back(exec->eval_stack.at(exec->eval_stack.size - static_cast<uint32_t>(i)));
  }
  exec->eval_stack.resize(exec->eval_stack.size - static_cast<uint32_t>(default_count));
  const auto* fn_meta = function_metadata_for_block(exec->mod->init_metadata, block_idx);
  std::string fn_name_str;
  if (fn_meta != nullptr) {
    fn_name_str = fn_meta->fn_name;
  } else {
    fn_name_str.assign(fn_name, fn_name_len);
  }
  std::vector<starlark::vm::frame*> closure;
  closure.reserve(exec->frame_chain.depth);
  for (uint32_t i = 0; i < exec->frame_chain.depth; ++i) {
    auto* captured = exec->frame_chain.data[i];
    closure.push_back(captured);
    pin_live_closure_frame(captured, exec);
  }
  auto* fn = google::protobuf::Arena::Create<native_starlark_function>(&ctx->arena(),
      fn_ptr,
      block_idx,
      fn_name_str,
      exec->mod->module_name,
      exec->mod,
      std::move(defaults),
      closure,
      exec->inner);
  eval_stack_push(exec, fn);
}

void starlark_obj_rt_assign_index_member(native_exec_context* exec, context* ctx, error_fn* err) {
  err = effective_err(exec, err);
  auto* index = exec->eval_stack.pop();
  auto* container = exec->eval_stack.pop();
  auto* element = exec->eval_stack.pop();
  tracked_error_bridge tracked(*err);
  container->index_assign(*index, *element, tracked);
  fail_exec_if_reported_error(exec, tracked.has_error(), err);
}
void starlark_obj_rt_assign_dot_member(native_exec_context* exec, const char* member, std::size_t len, context* ctx, error_fn* err) {
  err = effective_err(exec, err);
  auto* element = exec->eval_stack.pop();
  auto* value = exec->eval_stack.pop();
  element->dot_assign(std::string_view{member, len}, *value, *err);
}
void starlark_obj_rt_assign_slice_range(native_exec_context* exec, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* stride = exec->eval_stack.pop();
  auto* stop = exec->eval_stack.pop();
  auto* start = exec->eval_stack.pop();
  auto* container = exec->eval_stack.pop();
  auto* element = exec->eval_stack.pop();
  container->slice_range_assign(*start, *stop, *stride, *element, *ctx, *err);
}
void starlark_obj_rt_assign_slice_range_op(native_exec_context* exec, int32_t op_kind, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* rhs = exec->eval_stack.pop();
  auto* stride = exec->eval_stack.pop();
  auto* stop = exec->eval_stack.pop();
  auto* start = exec->eval_stack.pop();
  auto* container = exec->eval_stack.pop();
  slice_range_assign_op(nullptr, container, start, stop, stride, rhs, op_kind, *ctx, *err);
}

void starlark_obj_rt_frame_compound_assign(native_exec_context* exec, int32_t frame, int32_t slot, int32_t op_kind, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* frm = exec->frame_chain.at(static_cast<uint32_t>(frame));
  auto* value = frm->elements[static_cast<std::size_t>(slot)];
  if (value == nullptr) {
    report_exec_error(exec, err, starlark::error_messages::error_v2_unbound_variable(frm->names->Get(slot)));
    return;
  }
  auto* element = exec->eval_stack.pop();
  auto* result = compound_method(value, op_kind, element, *ctx, *err);
  if (result == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return;
  }
  frm->elements[static_cast<std::size_t>(slot)] = result;
}
void starlark_obj_rt_index_compound_assign(native_exec_context* exec, int32_t op_kind, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* rhs = exec->eval_stack.pop();
  auto* index = exec->eval_stack.pop();
  auto* container = exec->eval_stack.pop();
  auto* element = container->index(*index, *ctx, *err);
  if (element == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return;
  }
  auto* result = compound_method(element, op_kind, rhs, *ctx, *err);
  if (result == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return;
  }
  tracked_error_bridge tracked(*err);
  container->index_assign(*index, *result, tracked);
  fail_exec_if_reported_error(exec, tracked.has_error(), err);
}
void starlark_obj_rt_dot_compound_assign(native_exec_context* exec, const char* member, std::size_t len, int32_t op_kind, context* ctx, error_fn* err) {
  ctx = effective_ctx(exec, ctx);
  err = effective_err(exec, err);
  auto* rhs = exec->eval_stack.pop();
  auto* element = exec->eval_stack.pop();
  auto* field = element->dot(std::string_view{member, len}, *ctx, *err);
  if (field == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return;
  }
  auto* result = compound_method(field, op_kind, rhs, *ctx, *err);
  if (result == nullptr) {
    fail_exec_on_runtime_error(exec, err);
    return;
  }
  element->dot_assign(std::string_view{member, len}, *result, *err);
}

void starlark_obj_rt_load_module_symbol(native_exec_context* exec,
    const char* module_name,
    std::size_t module_len,
    const char* remote_symbol,
    std::size_t remote_len,
    int32_t frame,
    int32_t slot,
    error_fn* err) {
  err = effective_err(exec, err);
  auto mod_info = exec->mod->starlark_loader->load_module(std::string_view{module_name, module_len}, exec->mod->module_name);
  if (!mod_info.ok()) {
    report_exec_error(exec, err, starlark::error_messages::error_v2_unable_to_load_module(std::string_view{module_name, module_len}));
    return;
  }
  if (!(*mod_info)->ready()) {
    report_exec_error(exec, err, starlark::error_messages::error_v2_module_not_ready(std::string_view{module_name, module_len}));
    return;
  }
  auto* module_frame = (*mod_info)->get().first;
  bool found = false;
  for (int i = 0; i < module_frame->names->size(); ++i) {
    if (std::string_view{remote_symbol, remote_len} == module_frame->names->Get(i)) {
      exec->frame_chain.at(static_cast<uint32_t>(frame))->elements[static_cast<std::size_t>(slot)] = module_frame->elements[i];
      found = true;
      break;
    }
  }
  if (!found) {
    report_exec_error(exec,
        err,
        starlark::error_messages::error_v2_module_does_not_define_symbol(std::string_view{module_name, module_len},
            std::string_view{remote_symbol, remote_len}));
  }
}

}  // extern "C"

void retain_object_runtime_symbols_for_jit() {
  static void* const k_symbols[] = {
      reinterpret_cast<void*>(&starlark_obj_rt_create_int),
      reinterpret_cast<void*>(&starlark_obj_rt_create_float),
      reinterpret_cast<void*>(&starlark_obj_rt_get_const_string),
      reinterpret_cast<void*>(&starlark_obj_rt_load_frame_slot),
      reinterpret_cast<void*>(&starlark_obj_rt_ensure_stack_capacity),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_plus),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_minus),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_star),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_slash),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_slash_slash),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_percent),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_ampersand),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_pipe),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_hat),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_lshift),
      reinterpret_cast<void*>(&starlark_obj_rt_binary_rshift),
      reinterpret_cast<void*>(&starlark_obj_rt_unary_not),
      reinterpret_cast<void*>(&starlark_obj_rt_unary_plus),
      reinterpret_cast<void*>(&starlark_obj_rt_unary_minus),
      reinterpret_cast<void*>(&starlark_obj_rt_unary_tilde),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_eq),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_ne),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_lt),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_le),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_gt),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_ge),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_in),
      reinterpret_cast<void*>(&starlark_obj_rt_cmp_not_in),
      reinterpret_cast<void*>(&starlark_obj_rt_truthy),
      reinterpret_cast<void*>(&starlark_obj_rt_ctx_none_value),
      reinterpret_cast<void*>(&starlark_obj_rt_ctx_true_value),
      reinterpret_cast<void*>(&starlark_obj_rt_ctx_false_value),
      reinterpret_cast<void*>(&starlark_obj_rt_obj_numeric_type),
      reinterpret_cast<void*>(&starlark_obj_rt_obj_as_int64),
      reinterpret_cast<void*>(&starlark_obj_rt_dot),
      reinterpret_cast<void*>(&starlark_obj_rt_index),
      reinterpret_cast<void*>(&starlark_obj_rt_slice_range),
      reinterpret_cast<void*>(&starlark_obj_rt_call_pos0),
      reinterpret_cast<void*>(&starlark_obj_rt_call_pos1),
      reinterpret_cast<void*>(&starlark_obj_rt_call_pos2),
      reinterpret_cast<void*>(&starlark_obj_rt_call_pos3),
      reinterpret_cast<void*>(&starlark_obj_rt_call_pos),
      reinterpret_cast<void*>(&starlark_obj_rt_try_predeclared_call_pos),
      reinterpret_cast<void*>(&starlark_obj_rt_is_native_function),
      reinterpret_cast<void*>(&starlark_obj_rt_native_fn_direct_eligible),
      reinterpret_cast<void*>(&starlark_obj_rt_native_fn_code_ptr),
      reinterpret_cast<void*>(&starlark_obj_rt_call_native_direct),
      reinterpret_cast<void*>(&starlark_obj_rt_call_named),
      reinterpret_cast<void*>(&starlark_obj_rt_call_method_pos0),
      reinterpret_cast<void*>(&starlark_obj_rt_call_method_pos1),
      reinterpret_cast<void*>(&starlark_obj_rt_call_method_pos2),
      reinterpret_cast<void*>(&starlark_obj_rt_call_method_pos3),
      reinterpret_cast<void*>(&starlark_obj_rt_call_method_pos),
      reinterpret_cast<void*>(&starlark_obj_rt_call_pos_star),
      reinterpret_cast<void*>(&starlark_obj_rt_call_full),
      reinterpret_cast<void*>(&starlark_obj_rt_make_list),
      reinterpret_cast<void*>(&starlark_obj_rt_make_dict),
      reinterpret_cast<void*>(&starlark_obj_rt_add_to_list),
      reinterpret_cast<void*>(&starlark_obj_rt_add_to_dict),
      reinterpret_cast<void*>(&starlark_obj_rt_make_bigint),
      reinterpret_cast<void*>(&starlark_obj_rt_make_bytes),
      reinterpret_cast<void*>(&starlark_obj_rt_unpack),
      reinterpret_cast<void*>(&starlark_obj_rt_get_iterator),
      reinterpret_cast<void*>(&starlark_obj_rt_exec_frame_back_iterator),
      reinterpret_cast<void*>(&starlark_obj_rt_iterator_has_next),
      reinterpret_cast<void*>(&starlark_obj_rt_iterator_next_push),
      reinterpret_cast<void*>(&starlark_obj_rt_iterator_next_ext),
      reinterpret_cast<void*>(&starlark_obj_rt_end_iterator),
      reinterpret_cast<void*>(&starlark_obj_rt_make_tuple),
      reinterpret_cast<void*>(&starlark_obj_rt_make_native_function_meta),
      reinterpret_cast<void*>(&starlark_obj_rt_assign_index_member),
      reinterpret_cast<void*>(&starlark_obj_rt_assign_dot_member),
      reinterpret_cast<void*>(&starlark_obj_rt_assign_slice_range),
      reinterpret_cast<void*>(&starlark_obj_rt_assign_slice_range_op),
      reinterpret_cast<void*>(&starlark_obj_rt_frame_compound_assign),
      reinterpret_cast<void*>(&starlark_obj_rt_index_compound_assign),
      reinterpret_cast<void*>(&starlark_obj_rt_dot_compound_assign),
      reinterpret_cast<void*>(&starlark_obj_rt_load_module_symbol),
  };
  __attribute__((used)) static const void* const anchor = k_symbols;
  (void)anchor;
}

}  // namespace native
}  // namespace starlark
