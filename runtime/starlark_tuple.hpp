// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_TUPLE_HPP_
#define RUNTIME_STARLARK_TUPLE_HPP_

#include <string>
#include <vector>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_tuple : public starlark_obj {
 public:
  explicit starlark_tuple(int reserve_size = 0);
  std::string_view type() const override;
  starlark_tuple& add(starlark_obj* element);
  bool truthy() const override;
  void unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn* error_callback) override;
  bool binary_in(const starlark_obj& other, error_fn* error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn* error_callback) const override;
  void inner_freeze(std::vector<starlark_obj*>& to_freeze) override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  std::vector<starlark_obj*> values;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_TUPLE_HPP_

