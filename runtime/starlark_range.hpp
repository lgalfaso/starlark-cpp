// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_RANGE_HPP_
#define RUNTIME_STARLARK_RANGE_HPP_

#include <string>

#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_range : public starlark_obj {
 public:
  starlark_range();
  std::string_view type() const override;
  bool truthy() const override;
  void set_start(const starlark_obj* value);
  void set_end(const starlark_obj* value);
  void set_step(const starlark_obj* value);
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  int64_t len(error_fn& error_callback) const override;
  starlark_iterator* get_iterator(google::protobuf::Arena& arena, error_fn& error_callback) override;
  starlark_obj* index(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const override;

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

  uint64_t start;
  uint64_t end;
  uint64_t step;
  uint64_t last;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_RANGE_HPP_

