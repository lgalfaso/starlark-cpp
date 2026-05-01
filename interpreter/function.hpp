// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_FUNCTION_HPP_
#define INTERPRETER_FUNCTION_HPP_

#include <map>
#include <string>
#include <vector>

#include "interpreter/frame.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "runtime/starlark_function.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

class interpreter_function : public starlark::runtime::starlark_function {
 public:
  interpreter_function(
      int entrypoint,
      std::vector<starlark::runtime::starlark_obj*>&& default_arguments,
      const starlark::bytecode::FunctionSignature* function_signature,
      const google::protobuf::RepeatedPtrField<std::string>* frame_names,
      std::vector<std::vector<frame*>>& frame_stacks,
      std::vector<std::pair<int, int>>& call_stack,
      int& instruction_ptr,
      int& block_ptr);
  starlark::runtime::starlark_obj* call(
      const starlark::runtime::starlark_obj::pos_args_t& pos_args,
      const starlark::runtime::starlark_obj::named_args_t& named_args,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback) override;

 protected:
  bool inner_equals(starlark::runtime::equals_comparator& comp, const starlark::runtime::starlark_obj* other) const override;

 private:
  int entrypoint;
  std::vector<starlark::runtime::starlark_obj*> default_arguments;
  const starlark::bytecode::FunctionSignature* function_signature;
  std::map<std::string_view, std::size_t> named_argument_index;
  const google::protobuf::RepeatedPtrField<std::string>* frame_names;
  std::vector<frame*> frame_stack;
  std::vector<std::vector<frame*>>& frame_stacks;
  std::vector<std::pair<int, int>>& call_stack;
  int& instruction_ptr;
  int& block_ptr;
  starlark::runtime::starlark_obj* default_parameters;
};

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_FUNCTION_HPP_

