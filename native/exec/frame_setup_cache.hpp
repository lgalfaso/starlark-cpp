// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_FRAME_SETUP_CACHE_HPP_
#define NATIVE_FRAME_SETUP_CACHE_HPP_

#include <cstdint>

#include <vector>

#include "google/protobuf/repeated_field.h"
#include "native/exec/native_exec_context.hpp"
#include "native/exec/module_runtime_state.hpp"
#include "runtime/starlark_object.hpp"
#include "vm/module_metadata.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

using starlark_obj = starlark::runtime::starlark_obj;

void build_function_frame_cache(module_runtime_state& state);

const google::protobuf::RepeatedPtrField<std::string>* function_frame_names(module_runtime_state& mod_state,
    std::size_t fn_idx,
    starlark::runtime::context& ctx,
    const starlark::vm::function_signature_metadata& fn_meta);

starlark::vm::frame* acquire_function_frame(native_exec_context& exec,
    google::protobuf::Arena& arena,
    const google::protobuf::RepeatedPtrField<std::string>* names);

void recycle_top_function_frame(native_exec_context& exec);

void bind_positional_args_to_frame(starlark::vm::frame& fn_frame,
    const starlark::vm::function_signature_metadata& fn_meta,
    const native_function_frame_cache& cache,
    std::size_t fn_idx,
    starlark_obj** argv,
    int32_t argc);

void bind_positional_args_from_eval_stack(starlark::vm::frame& fn_frame,
    native_exec_context& exec,
    const starlark::vm::function_signature_metadata& fn_meta,
    const native_function_frame_cache& cache,
    std::size_t fn_idx,
    uint32_t callee_idx,
    int32_t argc);

bool setup_native_fn_frame_for_invoke(native_exec_context& exec,
    module_runtime_state& mod_state,
    int32_t block_idx,
    starlark_obj** argv,
    int32_t argc,
    bool from_eval_stack);

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_FRAME_SETUP_CACHE_HPP_
