// Copyright 2025 Lucas Mirelmann

#ifndef INTERPRETER_INTERPRETER_HPP_
#define INTERPRETER_INTERPRETER_HPP_

#include <string>
#include <string_view>
#include <vector>

#include "google/protobuf/arena.h"
#include "google/protobuf/repeated_field.h"
#include "logging/logging.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace interpreter {

struct frame {
  frame(std::size_t, const google::protobuf::RepeatedPtrField<std::string>* names);

  frame* parent_frame;
  std::vector<starlark::runtime::starlark_obj*> elements;
  const google::protobuf::RepeatedPtrField<std::string>* names;
};

class interpreter {
 public:
  interpreter();
  frame* run(std::string_view starlark_program, google::protobuf::Arena& arena,
      starlark::logging::logger& logging);
};

}  // namespace interpreter
}  // namespace starlark

#pragma GCC visibility pop

#endif  // INTERPRETER_INTERPRETER_HPP_

