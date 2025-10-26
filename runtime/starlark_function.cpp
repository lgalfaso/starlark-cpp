// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_function.hpp"

#include <format>
#include <map>
#include <string>
#include <vector>

namespace starlark {
namespace runtime {

starlark_built_in_function::starlark_built_in_function(fn* native_fn, const std::string& fn_name) :
  native_fn(native_fn), fn_name(fn_name) {}

std::string_view starlark_built_in_function::type() const {
  return "builtin_function_or_method";
}

bool starlark_built_in_function::inner_repr(printer& print, printer_action action) const {
  print.append(std::format("<built-in function {}>", fn_name));
  return false;
}

bool starlark_built_in_function::truthy() const {
  return true;
}

bool starlark_built_in_function::inner_equals(comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_built_in_function::inner_hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

starlark_obj* starlark_built_in_function::call(
    const std::vector<starlark_obj*>& pos_args,
    const std::map<std::string, starlark_obj*>& named_args) {
  return native_fn(pos_args, named_args);
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

std::variant<int64_t, starlark_obj::pending_hash> starlark_function::inner_hash() const {
  // TODO(lmirelmann): Implement.
  return 0;
}

}  // namespace runtime
}  // namespace starlark


