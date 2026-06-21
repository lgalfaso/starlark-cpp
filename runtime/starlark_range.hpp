// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_RANGE_HPP_
#define RUNTIME_STARLARK_RANGE_HPP_

#include <string>
#include <vector>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

struct range_state {
  const int64_t start;
  const int64_t end;
  const int64_t step;
  const int64_t len;
};

class starlark_range : public starlark_obj {
 public:
  starlark_range(int64_t start, int64_t end, int64_t step);
  std::string_view type() const override;
  bool truthy() const override;
  void unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, context& ctx, error_fn& error_callback) override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  int64_t len(bool produce_error, error_fn& error_callback) const override;
  starlark_iterator* get_iterator(bool produce_error, context& ctx, error_fn& error_callback) override;
  starlark_obj* index(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const override;

  class starlark_range_iterator : public starlark_iterator {
   public:
    starlark_range_iterator(int64_t current_pos, int64_t step, int64_t remaining, context& ctx);
    bool has_next() const override;
    starlark_obj* next() override;
    void end_iterator() override;

   private:
    int64_t current_pos;
    const int64_t step;
    int64_t remaining;
    context& ctx;
  };

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

  const range_state state;
};

range_state calculate_state(int64_t start, int64_t end, int64_t step);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_RANGE_HPP_

