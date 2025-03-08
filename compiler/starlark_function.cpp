// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_function.hpp"

#include <string>

namespace starlark {
namespace compiler {

const std::string starlark_built_in_function::type_value = "builtin_function_or_method";

const std::string& starlark_built_in_function::type() const {
  return type_value;
}

bool starlark_built_in_function::inner_repr(printer& print, uint64_t pos) const {
  // TODO(lmirelmann): Replace `FUNCTION_NAME` with the right name.
  print.append("<built-in function $FUNCTION_NAME>");
  return false;
}

bool starlark_built_in_function::truthy() const {
  return true;
}

bool starlark_built_in_function::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_built_in_function::hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

const std::string starlark_function::type_value = "function";

const std::string& starlark_function::type() const {
  return type_value;
}

bool starlark_function::inner_repr(printer& print, uint64_t pos) const {
  // TODO(lmirelmann): Replace `FUNCTION_NAME` and `MODULE` with the correct values. Eg:
  //     <function cc_fuzz_test from @@rules_fuzzing+//fuzzing/private:fuzz_test.bzl>
  //     <function _starlark_proto_encoder_rule_impl from //grammar:starlark_proto_encoder.bzl>
  print.append("<function $FUNCTION_NAME from $MODULE>");
  return false;
}

bool starlark_function::truthy() const {
  return true;
}

bool starlark_function::equals(const starlark_obj& other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_function::hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

}  // namespace compiler
}  // namespace starlark


