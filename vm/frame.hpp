// Copyright 2024-2026 Lucas Mirelmann

#ifndef VM_FRAME_HPP_
#define VM_FRAME_HPP_

#include <string>
#include <vector>

#include "google/protobuf/arena.h"
#include "google/protobuf/repeated_field.h"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

struct frame {
  explicit frame(const google::protobuf::RepeatedPtrField<std::string>* names);

  std::vector<starlark::runtime::starlark_obj*> elements;
  const google::protobuf::RepeatedPtrField<std::string>* names;
  std::vector<starlark::runtime::starlark_iterator*> iterators;
  // Set when a nested function captures this frame live; keeps pooled frames distinct across calls.
  bool pinned_for_closure = false;
};

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_FRAME_HPP_
