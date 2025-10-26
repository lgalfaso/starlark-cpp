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
  std::string_view type() const override;
  starlark_tuple& add(starlark_obj* element);
  bool truthy() const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  std::vector<const starlark_obj*> values;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_TUPLE_HPP_

