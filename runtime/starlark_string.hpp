// Copyright 2025-2026 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_STRING_HPP_
#define RUNTIME_STARLARK_STRING_HPP_

#include <cstdint>
#include <functional>
#include <map>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "runtime/starlark_object.hpp"
#include "runtime/starlark_range.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_string : public starlark_obj {
 public:
  explicit starlark_string(std::string&& value);
  explicit starlark_string(std::string_view value);
  explicit starlark_string(std::vector<char32_t>&& codepoints);
  explicit starlark_string(std::span<const char32_t> codepoints);
  explicit starlark_string(char32_t codepoint);
  bool primitive() const override;
  std::string str() const override;
  bool truthy() const override;
  const std::vector<std::string>& dir() const override;
  const std::map<std::string, fn*, std::less<>>& methods_meta() const override;
  starlark::result::status_or<int> cmp(const starlark_obj& other, std::string_view op, error_fn& error_callback) const override;
  bool binary_in(const starlark_obj& other, error_fn& error_callback) const override;
  starlark_obj* binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* binary_percent(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  starlark_obj* percent_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) override;
  int64_t unsafe_len() const override;
  starlark_obj* index(const starlark_obj& other, context& ctx, error_fn& error_callback) const override;
  starlark_obj* slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const override;
  std::string_view as_string() const override;

  int64_t count(std::span<const char32_t> sub, int64_t start, int64_t end) const;
  bool endswith(const std::vector<std::span<const char32_t>>& ends, int64_t start, int64_t end) const;
  bool startswith(const std::vector<std::span<const char32_t>>& begins, int64_t start, int64_t end) const;
  int64_t find(std::span<const char32_t> sub, int64_t start, int64_t end) const;
  int64_t rfind(std::span<const char32_t> sub, int64_t start, int64_t end) const;
  starlark_obj* join(const std::vector<std::span<const char32_t>>& elements, context& ctx) const;
  starlark_obj* partition(std::span<const char32_t> sub, context& ctx, error_fn& error_callback);
  starlark_obj* rpartition(std::span<const char32_t> sub, context& ctx, error_fn& error_callback);
  starlark_obj* replace(std::span<const char32_t> old, std::span<const char32_t> new_, int64_t count, context& ctx) const;
  starlark_obj* rsplit(int64_t maxsplit, context& ctx) const;
  starlark_obj* rsplit(std::span<const char32_t> sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const;
  starlark_obj* split(int64_t maxsplit, context& ctx) const;
  starlark_obj* split(std::span<const char32_t> sep, int64_t maxsplit, context& ctx, error_fn& error_callback) const;
  starlark_obj* lstrip(context& ctx) const;
  starlark_obj* lstrip(std::span<const char32_t> cutset, context& ctx) const;
  starlark_obj* rstrip(context& ctx) const;
  starlark_obj* rstrip(std::span<const char32_t> cutset, context& ctx) const;
  starlark_obj* strip(context& ctx) const;
  starlark_obj* strip(std::span<const char32_t> cutset, context& ctx) const;
  starlark_obj* splitlines(bool keepends, context& ctx) const;
  bool isalnum() const;
  bool isalpha() const;
  bool isdigit() const;
  bool isspace() const;
  bool islower() const;
  bool istitle() const;
  bool isupper() const;
  starlark_obj* lower(context& ctx);
  starlark_obj* title(context& ctx);
  starlark_obj* upper(context& ctx);
  starlark_obj* capitalize(context& ctx);
  starlark_obj* format(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);

  starlark_obj* removeprefix(std::span<const char32_t> sub, context& ctx);
  starlark_obj* removesuffix(std::span<const char32_t> sub, context& ctx);
  starlark_obj* elems(context& ctx) const;
  starlark_obj* elem_ords(context& ctx) const;
  starlark_obj* codepoints(context& ctx) const;
  starlark_obj* codepoint_ords(context& ctx) const;

  const std::vector<char32_t>& codepoint_data() const { return codepoints_; }

  class string_elems : public starlark_obj {
   public:
    string_elems(const starlark_string* str, range_state state, bool is_cp, bool ords);
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
    const starlark_string* str;
    const range_state state;
    const bool is_cp;
    const bool ords;
  };

  class starlark_elems_iterator : public starlark_iterator {
   public:
    starlark_elems_iterator(const starlark_string* str, int64_t current_pos, int64_t step, int64_t remaining, bool ords, context& ctx);
    bool has_next() const override;
    starlark_obj* next() override;
    void end_iterator() override;

   private:
    const starlark_string* str;
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
  void init_from_utf8(std::string_view value);
  void ensure_utf8_cache() const;
  char32_t codepoint_at(std::size_t element) const;

  std::vector<char32_t> codepoints_;
  mutable std::optional<std::string> utf8_cache_;
  mutable bool hash_computed = false;
  mutable int64_t cached_hash = 0;
};

void append_for_repr(std::string& output, std::string_view input);
void append_for_repr(std::string& output, char32_t code_point);
starlark::result::status chr_fn(std::string& output, int64_t input, error_fn& error_callback);
starlark::result::status chr_fn(std::string& output, const starlark::bigint::number& input, error_fn& error_callback);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_STRING_HPP_
