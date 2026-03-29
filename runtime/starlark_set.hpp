// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_SET_HPP_
#define RUNTIME_STARLARK_SET_HPP_

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "containers/linked_hash_set.hpp"
#include "runtime/starlark_object.hpp"
#include "status_or/status.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_set : public starlark_obj {
 public:
  typedef starlark::cnt::linked_hash_set<starlark_obj*, starlark_hash_op, starlark_equals_to> set_t;

  starlark_set();
  std::string_view type() const override;
  bool truthy() const override;
  const std::vector<std::string>& dir() const override;
  const std::map<std::string, fn*, std::less<>>& methods_meta() const override;
  bool contains(starlark_obj* obj) const;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  starlark_obj* binary_and(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_pipe(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_hat(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_minus(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* minus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* ampersand_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* pipe_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* hat_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  int64_t len(bool produce_error, error_fn& error_callback) const override;
  starlark_iterator* get_iterator(bool produce_error, context& ctx, error_fn& error_callback) override;

  starlark::result::status_or<bool> add(starlark_obj* element, error_fn& error_callback);
  starlark::result::status clear(error_fn& error_callback);
  starlark_obj* difference(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback) const;
  starlark::result::status difference_update(std::vector<starlark_obj*> others, context& ctx, error_fn& error_callback);
  starlark::result::status discard(starlark_obj* element, error_fn& error_callback);
  starlark_obj* intersection(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback) const;
  starlark::result::status intersection_update(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback);
  starlark::result::status_or<bool> isdisjoint(starlark_obj* other, context& ctx, error_fn& error_callback) const;
  starlark::result::status_or<bool> issubset(starlark_obj* other, context& ctx, error_fn& error_callback) const;
  starlark::result::status_or<bool> issuperset(starlark_obj* other, context& ctx, error_fn& error_callback) const;
  starlark_obj* pop(error_fn& error_callback);
  starlark::result::status remove(starlark_obj* element, error_fn& error_callback);
  starlark_obj* symmetric_difference(starlark_obj* other, context& ctx, error_fn& error_callback) const;
  starlark::result::status symmetric_difference_update(starlark_obj* other, context& ctx, error_fn& error_callback);
  starlark_obj* union_(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback);
  starlark::result::status update(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback);

  class starlark_set_iterator : public starlark_iterator {
   public:
    explicit starlark_set_iterator(starlark_set* set);
    bool has_next() const override;
    starlark_obj* next() override;
    void end_iterator() override;

   private:
    starlark_set* set;
    starlark::cnt::linked_hash_set<starlark_obj*, starlark_hash_op, starlark_equals_to>::iterator it;
  };

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_freeze(std::vector<starlark_obj*>& to_freeze) override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  bool can_modify(std::string_view op, error_fn& error_callback) const;

  static const std::map<std::string, fn*, std::less<>>& method_refs();
  static const std::vector<std::string>& attributes();

  set_t values;
  int iterators_count;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_SET_HPP_

