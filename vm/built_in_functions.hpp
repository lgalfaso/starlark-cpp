// Copyright 2026 Lucas Mirelmann

#ifndef VM_BUILT_IN_FUNCTIONS_HPP_
#define VM_BUILT_IN_FUNCTIONS_HPP_

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

starlark::runtime::starlark_obj* starlark_fn_inner_max(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
starlark::runtime::starlark_obj* starlark_fn_inner_min(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
starlark::runtime::starlark_obj* starlark_fn_inner_sorted(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_BUILT_IN_FUNCTIONS_HPP_
