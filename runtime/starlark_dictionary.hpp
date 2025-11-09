// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_DICTIONARY_HPP_
#define RUNTIME_STARLARK_DICTIONARY_HPP_

#include <string>

#include "containers/linked_hash_map.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_dictionary : public starlark_obj {
 public:
  std::string_view type() const override;
  bool truthy() const override;
  bool insert(starlark_obj* key, starlark_obj* value);

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  starlark::cnt::linked_hash_map<starlark_obj*, starlark_obj*, starlark_hash_op, starlark_equals_to> values;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_DICTIONARY_HPP_

