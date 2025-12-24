// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_DICTIONARY_HPP_
#define RUNTIME_STARLARK_DICTIONARY_HPP_

#include <string>

#include "containers/linked_hash_map.hpp"
#include "runtime/error_fn.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_dictionary : public starlark_obj {
 public:
  std::string_view type() const override;
  bool truthy() const override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  bool insert(starlark_obj* key, starlark_obj* value, error_fn& error_callback);
  starlark_obj* binary_pipe(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;
  int64_t len(error_fn& error_callback) const override;
  starlark_iterator* get_iterator(google::protobuf::Arena& arena, error_fn& error_callback) override;

  class starlark_dictionary_iterator : public starlark_iterator {
   public:
    starlark_dictionary_iterator(starlark_dictionary* dictionary);
    bool has_next() const override;
    starlark_obj* next() override;
    void end_iterator() override;

   private:
    starlark_dictionary* dictionary;
    starlark::cnt::linked_hash_map<starlark_obj*, starlark_obj*, starlark_hash_op, starlark_equals_to>::iterator it;
  };

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_freeze(std::vector<starlark_obj*>& to_freeze) override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  starlark::cnt::linked_hash_map<starlark_obj*, starlark_obj*, starlark_hash_op, starlark_equals_to> values;
  int iterators_count = 0;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_DICTIONARY_HPP_

