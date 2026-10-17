// Copyright 2026 Lucas Mirelmann

#ifndef VM_BUILTIN_STAR_TRAMPOLINE_HPP_
#define VM_BUILTIN_STAR_TRAMPOLINE_HPP_

#include <string_view>

#include "runtime/starlark_object.hpp"
#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

starlark::runtime::starlark_obj* call_builtin_star_function(module_loader& loader,
    std::string_view fn_name,
    const starlark::runtime::starlark_obj::pos_args_t& pos_args,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    starlark::runtime::context& ctx,
    starlark::runtime::error_fn& error_callback);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_BUILTIN_STAR_TRAMPOLINE_HPP_
