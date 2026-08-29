// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_RUNNER_STATE_HPP_
#define INTERPRETER_RUNNER_STATE_HPP_

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "google/protobuf/repeated_field.h"
#include "interpreter/frame.hpp"
#include "interpreter/module_loader.hpp"
#include "proto/starlark_bytecode.pb.h"
#include "runtime/starlark_object.hpp"
#include "runtime/starlark_string.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

class interpreter_function;
struct less_fn {
  bool operator()(const interpreter_function* lhs, const interpreter_function* rhs) const;
};

struct call_stack_entry {
  int block_ptr = 0;
  int instruction_ptr = 0;
  bool inner = false;
};

struct runner_state {
  struct program_info {
    starlark::bytecode::Program* bytecode;
    std::string module_name;
    std::vector<starlark::runtime::starlark_string*>* const_strings = nullptr;
  };
  // If recursion is not allowed, then it is possible to replace this with `std::vector<std::vector<frame*>*> frame_stacks;`.
  // Doing so would prevent the copying of a std::vector during a call. Given that there is a chance that recursion will be allowed,
  // this is kept as is. If at a future point in time this were to change, and recursion were never be allowed, then this can be revisited.
  std::vector<std::vector<frame*>> frame_stacks;
  std::vector<call_stack_entry> call_stack;
  std::vector<program_info> current_program_stack;
  int block_ptr = 0;
  int instruction_ptr = 0;
  bool inner = false;
  program_info current_program;
  const google::protobuf::RepeatedPtrField<starlark::bytecode::OpCode>* current_op_codes = nullptr;
  module_loader* loader = nullptr;
  std::vector<interpreter_function*> call_fns;
  std::map<interpreter_function*, int, less_fn> fns_in_stack;
  starlark::runtime::starlark_obj::pos_args_t call_pos_args;
  starlark::runtime::starlark_obj::named_args_t call_named_args;
};

inline void refresh_current_op_codes(runner_state& state) {
  state.current_op_codes = &state.current_program.bytecode->block(state.block_ptr).op_code();
}

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_RUNNER_STATE_HPP_

