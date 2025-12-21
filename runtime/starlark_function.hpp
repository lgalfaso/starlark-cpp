// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_FUNCTION_HPP_
#define RUNTIME_STARLARK_FUNCTION_HPP_

#include <map>
#include <string>
#include <vector>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_function : public starlark_obj {
 public:
  std::string_view type() const override;
  bool truthy() const override;
  starlark_obj* call(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;
};

class starlark_built_in_function : public starlark_obj {
 public:
  typedef starlark_obj* (fn)(const std::vector<starlark_obj*>&, const std::map<std::string, starlark_obj*>&, google::protobuf::Arena& arena, error_fn& error_callback);

  starlark_built_in_function(fn* native_fn, const std::string& fn_name);
  std::string_view type() const override;
  bool truthy() const override;
  starlark_obj* call(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback) override;

 protected:
  fn* native_fn;
  const std::string fn_name;

  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_FUNCTION_HPP_

