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

starlark_obj* starlark_fn_abs(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_all(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_any(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_bool(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_bytes(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_chr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_dict(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_dir(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_enumerate(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_fail(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_float(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_getattr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_hasattr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_hash(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_int(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_len(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_list(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_max(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_min(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_ord(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_print(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_range(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_repr(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_reversed(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_set(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_sorted(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_str(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_tuple(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_type(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
starlark_obj* starlark_fn_zip(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, google::protobuf::Arena& arena, error_fn& error_callback);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_FUNCTION_HPP_

