// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_STRING_HPP_
#define RUNTIME_STARLARK_STRING_HPP_

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_string : public starlark_obj {
 public:
  explicit starlark_string(std::string&& value);
  explicit starlark_string(std::string_view value);
  std::string_view type() const override;
  bool primitive() const override;
  std::string str() const override;
  bool truthy() const override;
  const std::vector<std::string>& dir() const override;
  const std::map<std::string, fn*, std::less<>>& methods_meta() const override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_percent(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* plus_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) override;
  starlark_obj* star_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) override;
  starlark_obj* percent_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) override;
  int64_t len(bool produce_error, error_fn& error_callback) const override;
  starlark_obj* index(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  std::string_view as_string() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  static const std::map<std::string, fn*, std::less<>>& method_refs();
  static const std::vector<std::string>& attributes();

  std::string value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_STRING_HPP_

