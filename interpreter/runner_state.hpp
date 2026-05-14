// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_RUNNER_STATE_HPP_
#define INTERPRETER_RUNNER_STATE_HPP_

#include <string>
#include <utility>
#include <vector>

#include "interpreter/frame.hpp"
#include "interpreter/module_loader.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

struct runner_state {
  // If recursion is not allowed, then it is possible to replace this with `std::vector<std::vector<frame*>*> frame_stacks;`.
  // Doing so would prevent the copying of a std::vector during a call. Given that there is a chance that recursion will be allowed,
  // this is kept as is. If at a future point in time this were to change, and recursion were never be allowed, then this can be revisited.
  std::vector<std::vector<frame*>> frame_stacks;
  std::vector<std::pair<int, int>> call_stack;
  std::vector<std::pair<starlark::bytecode::Program*, std::string>*> current_program_stack;
  int instruction_ptr = 0;
  int block_ptr = 0;
  std::pair<starlark::bytecode::Program*, std::string>* current_program = nullptr;
  module_loader* loader = nullptr;
  std::vector<starlark::runtime::starlark_obj*> call_fns;
};

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_RUNNER_STATE_HPP_

