// Copyright 2026 Lucas Mirelmann

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_object.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

starlark_obj* create_function(context& ctx, starlark_obj* this_obj, starlark_obj::fn native_fn, std::string_view fn_name) {
  return create_function(ctx, this_obj, builtin_entrypoints{.call = native_fn}, fn_name);
}

starlark_obj* create_function(context& ctx, starlark_obj* this_obj, builtin_entrypoints entrypoints, std::string_view fn_name) {
  return Arena::Create<starlark_built_in_function>(&ctx.arena(), this_obj, entrypoints, fn_name);
}

}  // namespace runtime
}  // namespace starlark
