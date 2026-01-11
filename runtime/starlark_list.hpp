// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_LIST_HPP_
#define RUNTIME_STARLARK_LIST_HPP_

#include <functional>
#include <map>
#include <string>
#include <vector>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_list : public starlark_obj {
 public:
  explicit starlark_list(std::size_t reserve_size);
  std::string_view type() const override;
  void add(starlark_obj* element, error_fn& error_callback);
  bool truthy() const override;
  const std::vector<std::string>& dir() const override;
  const std::map<std::string, fn*, std::less<>>& methods_meta() const override;
  void unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn& error_callback) override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  int64_t len(bool produce_error, error_fn& error_callback) const override;
  starlark_iterator* get_iterator(bool produce_error, google::protobuf::Arena& arena, error_fn& error_callback) override;
  starlark_obj* index(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  void index_assign(const starlark_obj& idx, starlark_obj& element, error_fn& error_callback) override;

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

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const override;
  void inner_freeze(std::vector<starlark_obj*>& to_freeze) override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  static const std::map<std::string, fn*, std::less<>> method_refs;
  static const std::vector<std::string> attributes;

  std::vector<starlark_obj*> values;
  int iterators_count = 0;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_LIST_HPP_

