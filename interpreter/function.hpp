// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_FUNCTION_HPP_
#define INTERPRETER_FUNCTION_HPP_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "interpreter/runner_state.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "runtime/starlark_function.hpp"
#include "vm/frame.hpp"

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
      std::string_view fn_name,
      const google::protobuf::RepeatedPtrField<std::string>* frame_names,
      const std::vector<starlark::vm::frame*>& frame_stack);
  starlark::runtime::starlark_obj* call(
      const starlark::runtime::starlark_obj::pos_args_t& pos_args,
      const starlark::runtime::starlark_obj::named_args_t& named_args,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback) override;
  starlark::runtime::starlark_obj* call_pos(
      std::span<starlark::runtime::starlark_obj*> pos_args,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback) override;

 private:
  starlark::runtime::starlark_obj* call_pos_general(
      std::span<starlark::runtime::starlark_obj*> pos_args,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback);

 public:
  starlark::runtime::starlark_obj* call_pos_fixed_0(starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
  starlark::runtime::starlark_obj* call_pos_fixed_1(
      starlark::runtime::starlark_obj* arg0,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback);
  starlark::runtime::starlark_obj* call_pos_fixed_2(
      starlark::runtime::starlark_obj* arg0,
      starlark::runtime::starlark_obj* arg1,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback);
  starlark::runtime::starlark_obj* call_pos_fixed_3(
      starlark::runtime::starlark_obj* arg0,
      starlark::runtime::starlark_obj* arg1,
      starlark::runtime::starlark_obj* arg2,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback);

 protected:
  bool inner_equals(starlark::runtime::equals_comparator& comp, const starlark::runtime::starlark_obj* other) const override;
  void inner_cmp(starlark::runtime::order_comparator& comp, const starlark::runtime::starlark_obj* other, std::string_view op, bool extended, starlark::runtime::error_fn& error_callback) const override;
  void inner_freeze(std::vector<starlark::runtime::starlark_obj*>& to_freeze) override;

 private:
  void compute_simple_call_metadata();
  bool begin_call(runner_state* state, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
  void commit_frame(starlark::vm::frame* new_frame, runner_state* state, starlark::runtime::context& ctx);
  void apply_keyword_only_defaults(starlark::vm::frame* new_frame);
  const starlark::vm::function_signature_metadata* fn_meta() const;

  int entrypoint;
  std::vector<starlark::runtime::starlark_obj*> default_arguments;
  std::map<std::string_view, std::size_t> named_argument_index;
  const google::protobuf::RepeatedPtrField<std::string>* frame_names;
  std::vector<starlark::vm::frame*> frame_stack;
  starlark::runtime::starlark_obj* default_parameters;
  runner_state::program_info current_program;
  const bool inner_;

  int simple_positional_arity_ = -1;
  std::vector<int> positional_param_frame_pos_;
  int keyword_only_param_start_ = 0;
  int keyword_only_param_count_ = 0;
  int keyword_only_default_offset_ = 0;

  friend std::strong_ordering cmp_fn(const interpreter_function* lhs, const interpreter_function* rhs, bool compare_stack);
};

starlark::runtime::starlark_obj* starlark_fn_max_impl(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
starlark::runtime::starlark_obj* starlark_fn_min_impl(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);
starlark::runtime::starlark_obj* starlark_fn_sorted_impl(starlark::runtime::starlark_obj* this_obj, const starlark::runtime::starlark_obj::pos_args_t& pos_args, const starlark::runtime::starlark_obj::named_args_t& named_args, starlark::runtime::context& ctx, starlark::runtime::error_fn& error_callback);

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_FUNCTION_HPP_
