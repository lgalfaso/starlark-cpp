// Copyright 2025 Lucas Mirelmann

#include "compiler/object.hpp"

#include <string>

namespace starlark {
namespace compiler {

starlark_obj::~starlark_obj() {}

size_t starlark_hash::operator()(const starlark_obj* value) const {
  return value->hash();
}

bool starlark_equals_to::operator()(const starlark_obj* lhs, const starlark_obj* rhs) const {
  return lhs->equals(*rhs);
}

}  // namespace compiler
}  // namespace starlark


