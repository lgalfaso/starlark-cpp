// Copyright 2026 Lucas Mirelmann

#ifndef INTERPRETER_FRAME_HPP_
#define INTERPRETER_FRAME_HPP_

#include <string>
#include <vector>

#include "google/protobuf/arena.h"
#include "google/protobuf/repeated_field.h"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

struct frame {
  frame(const google::protobuf::RepeatedPtrField<std::string>* names);

  std::vector<starlark::runtime::starlark_obj*> elements;
  const google::protobuf::RepeatedPtrField<std::string>* names;
  std::vector<starlark::runtime::starlark_iterator*> iterators;
};

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_FRAME_HPP_

