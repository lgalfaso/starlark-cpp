// Copyright 2026 Lucas Mirelmann

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_object.hpp"

using ::google::protobuf::Arena;

namespace starlark {
namespace runtime {

starlark_obj* create_function(context& ctx, starlark_obj* this_obj, starlark_obj::fn native_fn, std::string_view fn_name) {
  return Arena::Create<starlark_built_in_function>(&ctx.arena(), this_obj, native_fn, fn_name);
}

}  // namespace runtime
}  // namespace starlark

