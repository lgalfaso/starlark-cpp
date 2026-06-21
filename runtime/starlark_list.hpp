// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_LIST_HPP_
#define RUNTIME_STARLARK_LIST_HPP_

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "runtime/starlark_object.hpp"
#include "status_or/status.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_list : public starlark_obj {
 public:
  explicit starlark_list(std::size_t reserve_size);
  std::string_view type() const override;
  bool truthy() const override;
  const std::vector<std::string>& dir() const override;
  const std::map<std::string, fn*, std::less<>>& methods_meta() const override;
  void unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, context& ctx, error_fn& error_callback) override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  int64_t len(bool produce_error, error_fn& error_callback) const override;
  starlark_iterator* get_iterator(bool produce_error, context& ctx, error_fn& error_callback) override;
  starlark_obj* index(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  void index_assign(const starlark_obj& idx, starlark_obj& element, error_fn& error_callback) override;
  starlark_obj* slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const override;

  starlark::result::status append(starlark_obj* element, context& ctx, error_fn& error_callback);
  starlark::result::status clear(error_fn& error_callback);
  starlark::result::status extend(starlark_obj* other, context& ctx, error_fn& error_callback);
  starlark_obj* index(starlark_obj* element, int64_t start, int64_t end, context& ctx, error_fn& error_callback) const;
  starlark::result::status insert(starlark_obj* element, int64_t pos, error_fn& error_callback);
  starlark_obj* pop(int64_t idx, error_fn& error_callback);
  starlark::result::status remove(starlark_obj* element, error_fn& error_callback);

  class starlark_list_iterator : public starlark_iterator {
   public:
    explicit starlark_list_iterator(starlark_list* list);
    bool has_next() const override;
    starlark_obj* next() override;
    void end_iterator() override;

   private:
    starlark_list* list;
    std::vector<starlark_obj*>::iterator it;
  };

  void unsafe_append(starlark_obj* element);
  void unsafe_reverse();

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
  void inner_freeze(std::vector<starlark_obj*>& to_freeze) override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  bool can_modify(std::string_view op, error_fn& error_callback) const;

  static const std::map<std::string, fn*, std::less<>>& method_refs();
  static const std::vector<std::string>& attributes();

  std::vector<starlark_obj*> values;
  int iterators_count = 0;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_LIST_HPP_

