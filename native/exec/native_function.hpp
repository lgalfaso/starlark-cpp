// Copyright 2026 Lucas Mirelmann

#ifndef NATIVE_NATIVE_FUNCTION_HPP_
#define NATIVE_NATIVE_FUNCTION_HPP_

#include <map>
#include <string>
#include <vector>

#include "native/exec/module_runtime_state.hpp"
#include "runtime/starlark_function.hpp"
#include "vm/frame.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace native {

struct native_exec_context;

struct native_invoke_result {
  starlark::runtime::starlark_obj* value = nullptr;
  bool failed = false;
};

class native_starlark_function : public starlark::runtime::starlark_function {
 public:
  native_starlark_function(void* code_ptr,
      int32_t block_idx,
      std::string_view fn_name,
      std::string_view module_name,
      module_runtime_state* mod_state,
      std::vector<starlark::runtime::starlark_obj*> default_arguments,
      std::vector<starlark::vm::frame*> closure_frames,
      bool inner_fn);
  starlark::runtime::starlark_obj* call(const starlark::runtime::starlark_obj::pos_args_t& pos_args,
      const starlark::runtime::starlark_obj::named_args_t& named_args,
      starlark::runtime::context& ctx,
      starlark::runtime::error_fn& error_callback) override;

  bool inner_equals(starlark::runtime::equals_comparator& comp, const starlark::runtime::starlark_obj* other) const override;
  void inner_cmp(starlark::runtime::order_comparator& comp, const starlark::runtime::starlark_obj* other, std::string_view op, bool extended, starlark::runtime::error_fn& error_callback) const override;
  void inner_freeze(std::vector<starlark::runtime::starlark_obj*>& to_freeze) override;

  void* code_ptr() const { return code_ptr_; }
  int32_t block_idx() const { return block_idx_; }
  bool direct_dispatch_eligible() const { return direct_dispatch_eligible_; }
  module_runtime_state* mod_state() const { return mod_state_; }

  native_invoke_result invoke_jit(native_exec_context* exec,
      starlark::runtime::starlark_obj** argv,
      int32_t argc,
      bool args_on_eval_stack,
      starlark::runtime::context* ctx,
      starlark::runtime::error_fn* err,
      bool manage_recursion_stack = true);

 private:
  void* code_ptr_;
  int32_t block_idx_;
  module_runtime_state* mod_state_;
  bool inner_;
  bool direct_dispatch_eligible_ = false;
  std::vector<starlark::runtime::starlark_obj*> default_arguments_;
  std::vector<starlark::vm::frame*> closure_frames_;
  std::map<std::string, int, std::less<>> named_argument_index_;
};

}  // namespace native
}  // namespace starlark

#pragma GCC visibility pop

#endif  // NATIVE_NATIVE_FUNCTION_HPP_
