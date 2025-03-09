// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_function.hpp"

#include <string>

namespace starlark {
namespace compiler {

std::string_view starlark_built_in_function::type() const {
  return "builtin_function_or_method";
}

bool starlark_built_in_function::inner_repr(printer& print, printer_action action) const {
  // TODO(lmirelmann): Replace `FUNCTION_NAME` with the right name.
  print.append("<built-in function $FUNCTION_NAME>");
  return false;
}

bool starlark_built_in_function::truthy() const {
  return true;
}

bool starlark_built_in_function::inner_equals(comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_built_in_function::hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

std::string_view starlark_function::type() const {
  return "function";
}

bool starlark_function::inner_repr(printer& print, printer_action action) const {
  // TODO(lmirelmann): Replace `FUNCTION_NAME` and `MODULE` with the correct values. Eg:
  //     <function cc_fuzz_test from @@rules_fuzzing+//fuzzing/private:fuzz_test.bzl>
  //     <function _starlark_proto_encoder_rule_impl from //grammar:starlark_proto_encoder.bzl>
  print.append("<function $FUNCTION_NAME from $MODULE>");
  return false;
}

bool starlark_function::truthy() const {
  return true;
}

bool starlark_function::inner_equals(comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

int64_t starlark_function::hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

}  // namespace compiler
}  // namespace starlark


