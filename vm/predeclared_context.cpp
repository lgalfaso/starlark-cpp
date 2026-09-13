// Copyright 2026 Lucas Mirelmann

#include "vm/predeclared_context.hpp"

#include "runtime/starlark_function.hpp"

namespace starlark {
namespace vm {

using starlark::runtime::context;
using starlark::runtime::starlark_obj;

void add_core_predeclared_globals(std::map<std::string, starlark_obj*, std::less<>>& global_context, context& ctx) {
  global_context["True"] = ctx.true_value();
  global_context["False"] = ctx.false_value();
  global_context["None"] = ctx.none_value();
  starlark::runtime::add_predeclared_builtins(global_context, ctx);
}

}  // namespace vm
}  // namespace starlark
