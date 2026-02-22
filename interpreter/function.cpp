// Copyright 2026 Lucas Mirelmann

#include "interpreter/function.hpp"

#include <utility>
#include <vector>

using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::starlark_obj;

namespace starlark {
namespace interpreter {

interpreter_function::interpreter_function(int entrypoint, std::vector<std::vector<frame*>>& frame_stacks, std::vector<std::pair<int, int>>& call_stack, int& instruction_ptr, int& block_ptr) :
  entrypoint(entrypoint), frame_stack(frame_stacks.back()), frame_stacks(frame_stacks), call_stack(call_stack), instruction_ptr(instruction_ptr), block_ptr(block_ptr) {
  default_parameters = nullptr;
}


starlark_obj* interpreter_function::call(
      const starlark_obj::pos_args_t& pos_args,
      const starlark_obj::named_args_t& named_args,
      context& ctx,
      error_fn& error_callback) {
  // TODO(lmirelmann): Do all the needed on the parameters.
  frame_stacks.push_back(frame_stack);
  call_stack.push_back(std::make_pair(block_ptr, instruction_ptr));
  block_ptr = entrypoint;
  instruction_ptr = 0;
  return ctx.none_value();
}

void interpreter_function::set_default_parameters(starlark::runtime::starlark_obj* default_parameters) {
  this->default_parameters = default_parameters;
}

}  // namespace interpreter
}  // namespace starlark

