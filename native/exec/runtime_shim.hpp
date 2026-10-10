// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_EXEC_RUNTIME_SHIM_HPP_
#define NATIVE_EXEC_RUNTIME_SHIM_HPP_

#include <cstddef>
#include <cstdint>

#include "native/exec/module_runtime_state.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

extern "C" {

void* starlark_rt_module_init_begin(module_runtime_state* state, starlark::runtime::error_fn* err);
bool starlark_rt_module_init_end(native_exec_context* exec);

bool starlark_rt_exec_create_predeclared(native_exec_context* exec, module_runtime_state* state, starlark::runtime::context* ctx, starlark::runtime::error_fn* err);
void starlark_rt_exec_create_frame_meta(native_exec_context* exec, std::uint32_t meta_index);

bool starlark_rt_exec_jit_function_entry(native_exec_context* exec);

bool starlark_rt_exec_setup_native_fn_frame_from_eval(native_exec_context* exec,
    module_runtime_state* mod_state,
    int32_t block_idx,
    int32_t argc);

bool starlark_rt_exec_setup_native_fn_frame_from_argv(native_exec_context* exec,
    module_runtime_state* mod_state,
    int32_t block_idx,
    starlark::runtime::starlark_obj** argv,
    int32_t argc);

}  // extern "C"

void retain_runtime_symbols_for_jit();

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_EXEC_RUNTIME_SHIM_HPP_
