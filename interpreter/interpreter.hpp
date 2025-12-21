// Copyright 2025 Lucas Mirelmann

#ifndef INTERPRETER_INTERPRETER_HPP_
#define INTERPRETER_INTERPRETER_HPP_

#include <google/protobuf/arena.h>

#include <string_view>
#include <vector>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

struct frame {
  explicit frame(std::size_t);

  frame* parent_frame;
  std::vector<starlark::runtime::starlark_obj*> elements;
};

class interpreter {
 public:
  interpreter();
  frame* run(std::string_view starlark_program, google::protobuf::Arena& arena);
};

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_INTERPRETER_HPP_

