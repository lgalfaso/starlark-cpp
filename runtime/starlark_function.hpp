// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_FUNCTION_HPP_
#define RUNTIME_STARLARK_FUNCTION_HPP_

#include <map>
#include <span>
#include <string>
#include <vector>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

struct starlark_built_in_functions {
  static const char abs_f[];
  static const char all_f[];
  static const char any_f[];
  static const char bool_f[];
  static const char bytes_f[];
  static const char chr_f[];
  static const char dict_f[];
  static const char dir_f[];
  static const char enumerate_f[];
  static const char fail_f[];
  static const char float_f[];
  static const char getattr_f[];
  static const char hasattr_f[];
  static const char hash_f[];
  static const char int_f[];
  static const char len_f[];
  static const char list_f[];
  static const char max_f[];
  static const char min_f[];
  static const char ord_f[];
  static const char print_f[];
  static const char range_f[];
  static const char repr_f[];
  static const char reversed_f[];
  static const char set_f[];
  static const char sorted_f[];
  static const char str_f[];
  static const char tuple_f[];
  static const char type_f[];
  static const char zip_f[];
};

using pos0_fn = starlark_obj* (*)(starlark_obj* this_obj, context& ctx, error_fn& error_callback);
using pos1_fn = starlark_obj* (*)(starlark_obj* this_obj, starlark_obj* a0, context& ctx, error_fn& error_callback);
using pos2_fn = starlark_obj* (*)(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, context& ctx, error_fn& error_callback);
using pos3_fn = starlark_obj* (*)(starlark_obj* this_obj, starlark_obj* a0, starlark_obj* a1, starlark_obj* a2, context& ctx, error_fn& error_callback);

struct builtin_entrypoints {
  starlark_obj::fn* call = nullptr;
  pos0_fn pos0 = nullptr;
  pos1_fn pos1 = nullptr;
  pos2_fn pos2 = nullptr;
  pos3_fn pos3 = nullptr;
};

class starlark_function : public starlark_obj {
 public:
  bool truthy() const override;
  starlark_obj* call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) override = 0;

 protected:
  starlark_function(std::string_view fn_name, std::string_view module_name, object_kind kind);
  const std::string fn_name;
  const std::string module_name;

  bool inner_repr(printer& print, printer_action action) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;
};

class starlark_built_in_function : public starlark_obj {
 public:
  starlark_built_in_function(starlark_obj* this_obj, builtin_entrypoints entrypoints, std::string_view fn_name);
  bool truthy() const override;
  starlark_obj* call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) override;
  starlark_obj* call_pos(std::span<starlark_obj*> pos_args, context& ctx, error_fn& error_callback) override;

 protected:
  starlark_obj* const this_obj;
  const builtin_entrypoints entrypoints;
  const std::string fn_name;

  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;
};

starlark_obj* starlark_fn_abs(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_all(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_any(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_bool(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_bytes(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_chr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_dict(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_dir(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_enumerate(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_fail(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_float(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_getattr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_hasattr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_hash(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_int(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_len(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_list(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_max(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_min(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_ord(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_print(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_range(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_repr(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_reversed(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_set(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_sorted(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_str(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_tuple(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_type(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_fn_zip(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);

starlark_obj* create_function(context& ctx, starlark_obj* this_obj, starlark_obj::fn native_fn, std::string_view fn_name);
starlark_obj* create_function(context& ctx, starlark_obj* this_obj, builtin_entrypoints entrypoints, std::string_view fn_name);

void add_predeclared_builtins(std::map<std::string, starlark_obj*, std::less<>>& global_context, context& ctx);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_FUNCTION_HPP_

