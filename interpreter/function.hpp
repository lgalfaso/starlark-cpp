// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_FUNCTION_HPP_
#define INTERPRETER_FUNCTION_HPP_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "interpreter/frame.hpp"
#include "interpreter/runner_state.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "runtime/starlark_function.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

class interpreter_function;

std::strong_ordering cmp_fn(const interpreter_function* lhs, const interpreter_function* rhs);

class interpreter_function : public starlark::runtime::starlark_function {
 public:
  interpreter_function(
      int entrypoint,
      std::vector<starlark::runtime::starlark_obj*>&& default_arguments,
      const runner_state::program_info& current_program_,
      bool inner_,
      const google::protobuf::RepeatedPtrField<std::string>* frame_names,
      const std::vector<frame*>& frame_stack);
  starlark::runtime::starlark_obj* call(
      const starlark::runtime::starlark_obj::pos_args_t& pos_args,
      const starlark::runtime::starlark_obj::named_args_t& named_args,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback) override;

 protected:
  bool inner_equals(starlark::runtime::equals_comparator& comp, const starlark::runtime::starlark_obj* other) const override;
  void inner_cmp(starlark::runtime::order_comparator& comp, const starlark::runtime::starlark_obj* other, std::string_view op, bool extended, starlark::runtime::error_fn& error_callback) const override;
  void inner_freeze(std::vector<starlark::runtime::starlark_obj*>& to_freeze) override;

 private:
  int entrypoint;
  std::vector<starlark::runtime::starlark_obj*> default_arguments;
  std::map<std::string_view, std::size_t> named_argument_index;
  const google::protobuf::RepeatedPtrField<std::string>* frame_names;
  std::vector<frame*> frame_stack;
  starlark::runtime::starlark_obj* default_parameters;
  runner_state::program_info current_program;
  const bool inner_;

  friend std::strong_ordering cmp_fn(const interpreter_function* lhs, const interpreter_function* rhs, bool compare_stack);
};

starlark::runtime::starlark_obj* starlark_fn_max_impl(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
starlark::runtime::starlark_obj* starlark_fn_min_impl(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
starlark::runtime::starlark_obj* starlark_fn_sorted_impl(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_FUNCTION_HPP_

