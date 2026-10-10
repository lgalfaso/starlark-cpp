// Copyright 2026 Lucas Mirelmann

#ifndef VM_PREDECLARED_CONTEXT_HPP_
#define VM_PREDECLARED_CONTEXT_HPP_

#include <map>
#include <string>
#include <functional>

#include "runtime/starlark_object.hpp"
#include "runtime/starlark_types.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

void add_core_predeclared_globals(std::map<std::string, starlark::runtime::starlark_obj*, std::less<>>& global_context,
    starlark::runtime::context& ctx);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_PREDECLARED_CONTEXT_HPP_
