// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_BYTES_HPP_
#define RUNTIME_STARLARK_BYTES_HPP_

#include <functional>
#include <map>
#include <string>
#include <string_view>
#include <vector>

#include "runtime/starlark_object.hpp"
#include "runtime/starlark_range.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_bytes : public starlark_obj {
 public:
  explicit starlark_bytes(std::string&& value);
  explicit starlark_bytes(std::string_view value);
  bool primitive() const override;
  bool truthy() const override;
  const std::vector<std::string>& dir() const override;
  const std::map<std::string, fn*, std::less<>>& methods_meta() const override;
  starlark::result::status_or<int> cmp(const starlark_obj& other, std::string_view op, error_fn& error_callback) const override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  int64_t unsafe_len() const override;
  starlark_obj* index(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const override;
  std::string_view as_string() const override;

  int64_t count(std::string_view sub, int64_t start, int64_t end) const;
  starlark_obj* elems(context& ctx) const;
  starlark_obj* elem_ords(context& ctx) const;
  bool endswith(const std::vector<std::string_view>& ends, int64_t start, int64_t end) const;
  int64_t find(std::string_view sub, int64_t start, int64_t end) const;
  starlark_obj* join(const std::vector<std::string_view>& elements, context& ctx) const;
  starlark_obj* lstrip(context& ctx) const;
  starlark_obj* lstrip(std::string_view cutset, context& ctx) const;
  starlark_obj* partition(std::string_view sub, context& ctx, error_fn& error_callback);
  starlark_obj* replace(std::string_view old, std::string_view new_, int64_t count, context& ctx) const;
  int64_t rfind(std::string_view sub, int64_t start, int64_t end) const;
  starlark_obj* rpartition(std::string_view sub, context& ctx, error_fn& error_callback);
  starlark_obj* rsplit(int64_t maxsplit, context& ctx) const;
  starlark_obj* rsplit(std::string_view sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const;
  starlark_obj* rstrip(context& ctx) const;
  starlark_obj* rstrip(std::string_view cutset, context& ctx) const;
  starlark_obj* split(int64_t maxsplit, context& ctx) const;
  starlark_obj* split(std::string_view sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const;
  bool startswith(const std::vector<std::string_view>& begins, int64_t start, int64_t end) const;
  starlark_obj* strip(context& ctx) const;
  starlark_obj* strip(std::string_view cutset, context& ctx) const;

  class bytes_elems : public starlark_obj {
   public:
    bytes_elems(const starlark_bytes* bytes, range_state state, bool ords);
    bool truthy() const override;

    bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
    int64_t unsafe_len() const override;
    starlark_iterator* get_iterator(bool produce_error, context& ctx, error_fn& error_callback) override;
    starlark_obj* index(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
    starlark_obj* slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const override;

   protected:
    bool inner_repr(printer& print, printer_action action) const override;
    bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
    void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
    std::variant<int64_t, pending_hash> inner_hash() const override;

   private:
    const starlark_bytes* bytes;
    const range_state state;
    const bool ords;
  };

  class starlark_elems_iterator : public starlark_iterator {
   public:
    starlark_elems_iterator(const starlark_bytes* bytes, int64_t current_pos, int64_t step, int64_t remaining, bool ords, context& ctx);
    bool has_next() const override;
    starlark_obj* next() override;
    void end_iterator() override;

   private:
    const starlark_bytes* bytes;
    int64_t current_pos;
    const int64_t step;
    int64_t remaining;
    const bool ords;
    context& ctx;
  };

 protected:
  bool inner_repr(printer& print, printer_action action) const override;
  bool inner_equals(equals_comparator& comp, const starlark_obj* other) const override;
  void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const override;
  std::variant<int64_t, pending_hash> inner_hash() const override;

 private:
  static const std::map<std::string, fn*, std::less<>>& method_refs();
  static const std::vector<std::string>& attributes();

  std::string value;
};

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_BYTES_HPP_

