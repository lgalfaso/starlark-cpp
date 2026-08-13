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
  explicit starlark_tuple(std::size_t reserve_size);
  std::string_view type() const override;
  starlark_tuple& add(starlark_obj* element);
  bool truthy() const override;
  void unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, context& ctx, error_fn& error_callback) override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  int64_t unsafe_len() const override;
  starlark_iterator* get_iterator(bool produce_error, context& ctx, error_fn& error_callback) override;
  starlark_obj* index(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const override;

  class starlark_tuple_iterator : public starlark_iterator {
   public:
    explicit starlark_tuple_iterator(starlark_tuple* tuple);
    bool has_next() const override;
    starlark_obj* next() override;
    void end_iterator() override;

   private:
    starlark_tuple* tuple;
    std::vector<starlark_obj*>::iterator it;
  };

  std::size_t size() const;
  const starlark_obj* at(std::size_t pos) const;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
  void inner_freeze(std::vector<starlark_obj*>& to_freeze) override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  std::vector<starlark_obj*> values;

  friend const std::vector<starlark_obj*>& inspect_tuple(const starlark_tuple& tuple);
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_TUPLE_HPP_

