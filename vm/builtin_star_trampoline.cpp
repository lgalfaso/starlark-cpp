// Copyright 2026 Lucas Mirelmann

#include "vm/builtin_star_trampoline.hpp"

#include "errors/runtime_error_messages.hpp"

using ::starlark::error_messages::error_v2_module_does_not_define_symbol;
using ::starlark::error_messages::error_v2_unable_to_load_module;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_obj;

namespace starlark {
namespace vm {

starlark_obj* call_builtin_star_function(module_loader& loader,
    std::string_view fn_name,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    context& ctx,
    error_fn& error_callback) {
  auto mod_info = loader.load_module(builtin_star_module);
  if (!mod_info.ok() || !(*mod_info)->ready()) {
    error_callback.add_error(error_v2_unable_to_load_module(builtin_star_module));
    return nullptr;
  }
  auto* builtin_frame = (*mod_info)->get().first;
  for (int i = 0; i < builtin_frame->names->size(); ++i) {
    if (builtin_frame->names->Get(i) == fn_name) {
      return builtin_frame->elements[static_cast<std::size_t>(i)]->call(pos_args, named_args, ctx, error_callback);
    }
  }
  error_callback.add_error(error_v2_module_does_not_define_symbol(builtin_star_module, fn_name));
  return nullptr;
}

}  // namespace vm
}  // namespace starlark
