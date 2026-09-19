// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_OBJECT_RUNTIME_HPP_
#define NATIVE_OBJECT_RUNTIME_HPP_

#include <cstddef>
#include <cstdint>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct native_exec_context;

extern "C" {

starlark::runtime::starlark_obj* starlark_obj_rt_create_int(starlark::runtime::context* ctx, int64_t value);
starlark::runtime::starlark_obj* starlark_obj_rt_create_float(starlark::runtime::context* ctx, double value);
starlark::runtime::starlark_obj* starlark_obj_rt_get_const_string(native_exec_context* exec, int64_t index);

void starlark_obj_rt_binary_plus(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_minus(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_star(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_slash(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_slash_slash(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_percent(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_ampersand(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_pipe(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_hat(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_lshift(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_binary_rshift(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);

void starlark_obj_rt_unary_not(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_unary_plus(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_unary_minus(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_unary_tilde(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);

void starlark_obj_rt_cmp_eq(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_cmp_ne(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_cmp_lt(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_cmp_le(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_cmp_gt(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_cmp_ge(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_cmp_in(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_cmp_not_in(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);

bool starlark_obj_rt_truthy(starlark::runtime::starlark_obj* value);

starlark::runtime::starlark_obj* starlark_obj_rt_ctx_none_value(starlark::runtime::context* ctx);
starlark::runtime::starlark_obj* starlark_obj_rt_ctx_true_value(starlark::runtime::context* ctx);
starlark::runtime::starlark_obj* starlark_obj_rt_ctx_false_value(starlark::runtime::context* ctx);
int32_t starlark_obj_rt_obj_numeric_type(starlark::runtime::starlark_obj* value);
int64_t starlark_obj_rt_obj_as_int64(starlark::runtime::starlark_obj* value);

void starlark_obj_rt_dot(native_exec_context* exec, const char* member, std::size_t len, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_index(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_slice_range(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);

void starlark_obj_rt_call_pos0(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_pos1(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_pos2(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_pos3(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_pos(native_exec_context* exec, int32_t count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
bool starlark_obj_rt_try_predeclared_call_pos(native_exec_context* exec,
    int32_t pos_count,
    starlark::runtime::context* ctx,
    starlark::runtime::error_fn* err);

bool starlark_obj_rt_is_native_function(starlark::runtime::starlark_obj* value);
void* starlark_obj_rt_native_fn_code_ptr(starlark::runtime::starlark_obj* value);
bool starlark_obj_rt_native_fn_direct_eligible(starlark::runtime::starlark_obj* value);
void starlark_obj_rt_call_native_direct(native_exec_context* exec, int32_t pos_count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_named(native_exec_context* exec, int32_t pos_count, int32_t named_count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);

void starlark_obj_rt_make_list(native_exec_context* exec, int32_t reserve, starlark::runtime::context* ctx);
void starlark_obj_rt_make_dict(native_exec_context* exec, starlark::runtime::context* ctx);
void starlark_obj_rt_add_to_list(native_exec_context* exec, int32_t count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_add_to_dict(native_exec_context* exec, int32_t count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_make_bigint(native_exec_context* exec, const char* digits, starlark::runtime::context* ctx);
void starlark_obj_rt_make_bytes(native_exec_context* exec, const char* data, std::size_t len, starlark::runtime::context* ctx);

void starlark_obj_rt_unpack(native_exec_context* exec, int32_t count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_get_iterator(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_end_iterator(native_exec_context* exec);
void* starlark_obj_rt_exec_frame_back_iterator(native_exec_context* exec);
bool starlark_obj_rt_iterator_has_next(void* iterator);
bool starlark_obj_rt_iterator_next_push(native_exec_context* exec, void* iterator);
bool starlark_obj_rt_iterator_next_ext(void* iterator);

void starlark_obj_rt_make_tuple(native_exec_context* exec, int32_t count, starlark::runtime::context* ctx);
void starlark_obj_rt_make_native_function_meta(native_exec_context* exec,
    void* fn_ptr,
    const char* fn_name,
    std::size_t fn_name_len,
    int32_t block_idx,
    int32_t default_count);

starlark::runtime::starlark_obj* starlark_obj_rt_load_frame_slot(native_exec_context* exec,
    int32_t frame,
    int32_t slot,
    starlark::runtime::error_fn* err);
void starlark_obj_rt_ensure_stack_capacity(native_exec_context* exec, int32_t required_capacity);

void starlark_obj_rt_call_method_pos0(native_exec_context* exec, const char* member, std::size_t len, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_method_pos1(native_exec_context* exec, const char* member, std::size_t len, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_method_pos2(native_exec_context* exec, const char* member, std::size_t len, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_method_pos3(native_exec_context* exec, const char* member, std::size_t len, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_method_pos(native_exec_context* exec, const char* member, std::size_t len, int32_t count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_pos_star(native_exec_context* exec, int32_t pos_count, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_call_full(native_exec_context* exec,
    int32_t positional_count,
    int32_t named_count,
    bool has_var_pos,
    bool has_var_named,
    starlark::runtime::context* ctx,
    starlark::runtime::error_fn* err);

void starlark_obj_rt_assign_index_member(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_assign_dot_member(native_exec_context* exec, const char* member, std::size_t len, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_assign_slice_range(native_exec_context* exec, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_assign_slice_range_op(native_exec_context* exec, int32_t op_kind, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);

void starlark_obj_rt_frame_compound_assign(native_exec_context* exec, int32_t frame, int32_t slot, int32_t op_kind, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_index_compound_assign(native_exec_context* exec, int32_t op_kind, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_obj_rt_dot_compound_assign(native_exec_context* exec, const char* member, std::size_t len, int32_t op_kind, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);

void starlark_obj_rt_load_module_symbol(native_exec_context* exec,
    const char* module_name,
    std::size_t module_len,
    const char* remote_symbol,
    std::size_t remote_len,
    int32_t frame,
    int32_t slot,
    starlark::runtime::error_fn* err);

}  // extern "C"

void retain_object_runtime_symbols_for_jit();

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_OBJECT_RUNTIME_HPP_
