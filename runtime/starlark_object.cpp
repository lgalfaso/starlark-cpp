// Copyright 2026 Lucas Mirelmann

#include "runtime/starlark_object.hpp"

#include <stdckdint.h>

#include <algorithm>
#include <bit>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "runtime/error_messages.hpp"
#include "runtime/levenshtein.hpp"
#include "runtime/starlark_types.hpp"

using ::starlark::bigint::number;
using ::starlark::result::error_status;
using ::starlark::result::ok_status;
using ::starlark::result::status;
using ::starlark::result::status_code;
using ::starlark::result::status_or;

namespace starlark {
namespace runtime {

void printer::append(std::string_view str) {
  partial_value += str;
}

const std::string& printer::value() const {
  return partial_value;
}

void printer::add_task(pending_task&& task) {
  tasks.emplace_back(std::move(task));
}

void printer::run() {
  while (!tasks.empty()) {
    auto top = tasks.back();
    tasks.pop_back();
    if (top.action == printer_action::kPrintTop && !stack.insert(top.obj).second) {
      top.obj->inner_repr(*this, printer_action::kPrintRecursion);
      continue;
    }
    if (!top.obj->inner_repr(*this, top.action)) {
      stack.erase(top.obj);
    }
  }
}

void equals_comparator::add_task(pending_task&& task) {
  tasks.emplace_back(std::move(task));
}

bool equals_comparator::run() {
  while (!tasks.empty()) {
    auto top = tasks.back();
    tasks.pop_back();
    if (top.lhs == top.rhs) {
      continue;
    }
    if (executed_tasks.insert(top).second) {
      if (!top.lhs->inner_equals(*this, top.rhs)) {
        return false;
      }
    }
  }
  return true;
}

size_t equals_comparator::pending_task_hash::operator()(const pending_task task) const {
  return hash_fn(task.lhs) ^ hash_fn(task.rhs);
}

bool equals_comparator::pending_task_equals_to::operator()(const pending_task& lhs, const pending_task& rhs) const {
  return (lhs.lhs == rhs.lhs && lhs.rhs == rhs.rhs) ||
         (lhs.lhs == rhs.rhs && lhs.rhs == rhs.lhs);
}

void order_comparator::add_task(pending_task_type task_type) {
  tasks.emplace_back(order_comparator::pending_task{
      .type = task_type,
  });
}

void order_comparator::add_task(const starlark_obj* lhs, const starlark_obj* rhs) {
  tasks.emplace_back(order_comparator::pending_task{
      .type = order_comparator::pending_task_type::kEvaluate,
      .lhs = lhs,
      .rhs = rhs,
  });
}

status_or<int> order_comparator::run(std::string_view op, error_fn& error_callback) {
  while (!tasks.empty()) {
    auto top = tasks.back();
    tasks.pop_back();
    switch (top.type) {
      case pending_task_type::kEvaluate:
        if (executed_tasks.insert(top).second) {
          top.lhs->inner_cmp(*this, top.rhs, op, error_callback);
        }
        break;
      case pending_task_type::kLessThan:
        return status_or<int>(-1);
      case pending_task_type::kGreaterThan:
        return status_or<int>(1);
      case pending_task_type::kFail:
        return status_or<int>(status_code::kError);
    }
  }
  return status_or<int>(0);
}

size_t order_comparator::pending_task_hash::operator()(const pending_task task) const {
  return hash_fn(task.lhs) ^ hash_fn(task.rhs);
}

bool order_comparator::pending_task_equals_to::operator()(const pending_task& lhs, const pending_task& rhs) const {
  return (lhs.lhs == rhs.lhs && lhs.rhs == rhs.rhs);
}

starlark_iterator::~starlark_iterator() {}

const std::vector<std::string>& starlark_obj::attributes() {
  static const std::vector<std::string>* result =
    new std::vector<std::string>();

  return *result;
}

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_obj::method_refs() {
  static const std::map<std::string, starlark_obj::fn*, std::less<>>* result =
    new std::map<std::string, starlark_obj::fn*, std::less<>>();

  return *result;
}

starlark_obj::starlark_obj() : freezed(false) {}

starlark_obj::~starlark_obj() {}

std::string starlark_obj::str() const {
  return repr();
}

std::string starlark_obj::repr() const {
  printer print;
  print.add_task(printer::pending_task{
    .obj = this,
    .action = printer_action::kPrintTop,
  });
  print.run();
  return print.value();
}

bool starlark_obj::primitive() const {
  return false;
}

const std::vector<std::string>& starlark_obj::dir() const {
  return attributes();
}

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_obj::methods_meta() const {
  return method_refs();
}

bool starlark_obj::equals(const starlark_obj& other) const {
  equals_comparator cmp;
  cmp.add_task(equals_comparator::pending_task{
    .lhs = this,
    .rhs = &other,
  });
  return cmp.run();
}

status_or<int> starlark_obj::cmp(const starlark_obj& other, std::string_view op, error_fn& error_callback) const {
  order_comparator cmp;
  cmp.add_task(this, &other);
  return cmp.run(op, error_callback);
}

int64_t starlark_obj::hash() const {
  // Replace this with -1 to disallow the hashing of objects that contain themself.
  const int64_t recursion_replacement = 1609587929392839161L;

  auto candidate = inner_hash();
  if (std::holds_alternative<int64_t>(candidate)) {
    return std::get<int64_t>(candidate);
  }

  // This implementation is recursion-free.
  std::vector<std::vector<const starlark_obj*>> pending;
  std::vector<std::vector<int64_t>> done;
  std::map<const starlark_obj*, int64_t> cache;
  std::vector<const starlark_obj*> current;

  pending_hash pending_hash_candidate = std::get<pending_hash>(candidate);
  pending.emplace_back(pending_hash_candidate.rbegin(), pending_hash_candidate.rend());
  done.emplace_back();
  current.push_back(this);
  while (true) {
    if (pending.back().empty()) {
      pending.pop_back();
      int64_t new_hash = starlark_hash(std::span<int64_t>(done.back().begin(), done.back().end()));
      done.pop_back();
      cache[current.back()] = new_hash;
      current.pop_back();
      if (done.empty()) {
        return new_hash;
      }
      done.back().emplace_back(new_hash);
    } else {
      const starlark_obj* element = pending.back().back();
      pending.back().pop_back();
      if (cache.contains(element)) {
        int64_t new_hash = cache[element];
        if (new_hash == -1) {
          return new_hash;
        }
        done.back().emplace_back(new_hash);
      } else {
        // This is added to detect hash recursions.
        cache[element] = recursion_replacement;
        candidate = element->inner_hash();
        if (std::holds_alternative<int64_t>(candidate)) {
          int64_t new_hash = std::get<int64_t>(candidate);
          if (new_hash == -1) {
            return new_hash;
          }
          done.back().emplace_back(new_hash);
          cache[element] = new_hash;
        } else {
          pending_hash pending_hash_candidate = std::get<pending_hash>(candidate);
          pending.emplace_back(pending_hash_candidate.rbegin(), pending_hash_candidate.rend());
          done.emplace_back();
          current.push_back(element);
        }
      }
    }
  }
}

void starlark_obj::freeze() {
  if (freezed) {
    return;
  }
  freezed = true;
  std::vector<starlark_obj*> to_freeze;
  inner_freeze(to_freeze);
  while (!to_freeze.empty()) {
    auto* element = to_freeze.back();
    to_freeze.pop_back();
    if (element->freezed) {
      continue;
    }
    element->freezed = true;
    element->inner_freeze(to_freeze);
  }
}

starlark_obj* starlark_obj::call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_not_callable(type()));
  return nullptr;
}

void starlark_obj::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn& error_callback) {
  error_callback.add_error(error_unpackable(type()));
}

starlark_obj* starlark_obj::unary_plus(context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_unary("+", type()));
  return nullptr;
}

starlark_obj* starlark_obj::unary_minus(context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_unary("-", type()));
  return nullptr;
}

starlark_obj* starlark_obj::unary_tilde(context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_unary("~", type()));
  return nullptr;
}

bool starlark_obj::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  error_callback.add_error(error_argument_uniterable(type()));
  return false;
}

starlark_obj* starlark_obj::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("+=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::minus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("-=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("*=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("/=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::slash_slash_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("//=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::percent_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("%=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::ampersand_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("&=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::pipe_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("|=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::hat_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("^=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::less_less_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary("<<=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::greater_greater_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_bad_operand_binary(">>=", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_lshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("<<", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_rshift(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary(">>", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_and(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("&", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_pipe(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("|", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_hat(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("^", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("+", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_minus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("-", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("*", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("/", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_slash_slash(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("//", type(), other.type()));
  return nullptr;
}

starlark_obj* starlark_obj::binary_percent(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_bad_operand_binary("%", type(), other.type()));
  return nullptr;
}

int64_t starlark_obj::len(bool produce_error, error_fn& error_callback) const {
  if (produce_error) {
    error_callback.add_error(error_no_method(type(), "len"));
  }
  return -1;
}

starlark_iterator* starlark_obj::get_iterator(bool produce_error, context& ctx, error_fn& error_callback) {
  if (produce_error) {
    error_callback.add_error(error_uniterable(type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_unsubscriptable(type()));
  return nullptr;
}

void starlark_obj::index_assign(const starlark_obj& idx, starlark_obj& element, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

starlark_obj* starlark_obj::dot(std::string_view field_name, context& ctx, error_fn& error_callback) {
  return get_attr(true, field_name, ctx, error_callback);
}

void starlark_obj::dot_assign(std::string_view field_name, starlark_obj& element, error_fn& error_callback) {
  auto& method_fns = methods_meta();
  auto it = method_fns.find(field_name);
  if (it == method_fns.end()) {
    auto& attributes = dir();
    auto candidate = levenshtein(field_name, attributes);
    if (candidate < 0) {
      error_callback.add_error(error_no_attribute(type(), field_name));
    } else {
      error_callback.add_error(error_no_attribute(type(), field_name, attributes[candidate]));
    }
  } else {
    error_callback.add_error(error_read_only_attribute(type(), field_name));
  }
}

starlark_obj* starlark_obj::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
  error_callback.add_error(error_unsubscriptable(type()));
  return nullptr;
}

void starlark_obj::slice_range_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_plus_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_minus_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_star_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_slash_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_slash_slash_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_percent_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_ampersand_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_pipe_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_hat_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_less_less_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

void starlark_obj::slice_range_greater_greater_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, context& ctx, error_fn& error_callback) {
  error_callback.add_error(error_no_item_assignment(type()));
}

int64_t starlark_obj::as_int64() const {
  return 0;
}

const number& starlark_obj::as_bigint() const {
  return number::zero();
}

double starlark_obj::as_float() const {
  return 0;
}

std::string_view starlark_obj::as_string() const {
  return "";
}

starlark_obj* starlark_obj::get_attr(bool produce_error, std::string_view attribute, context& ctx, error_fn& error_callback) {
  auto& method_fns = methods_meta();
  auto it = method_fns.find(attribute);
  if (it == method_fns.end()) {
    if (produce_error) {
      auto& attributes = dir();
      auto candidate = levenshtein(attribute, attributes);
      if (candidate < 0) {
        error_callback.add_error(error_no_attribute(type(), attribute));
      } else {
        error_callback.add_error(error_no_attribute(type(), attribute, attributes[candidate]));
      }
    }
    return nullptr;
  }
  return create_function(ctx, this, it->second, attribute);
}

starlark_numeric_type starlark_obj::numeric_type() const {
  return starlark_numeric_type::kNotNumeric;
}

void starlark_obj::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  error_callback.add_error(error_incomparable(op, type(), other->type()));
  comp.add_task(order_comparator::pending_task_type::kFail);
}

void starlark_obj::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  return;
}

status_or<int64_t> starlark_obj::inner_index(const starlark_obj& other, int64_t obj_len, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      auto idx = other.as_int64();
      if (idx < 0) {
        idx += obj_len;
      }
      if (idx < 0 || obj_len <= idx) {
        error_callback.add_error(error_index_out_of_range(type()));
        return status_or<int64_t>(status_code::kError);
      }
      return status_or<int64_t>(idx);
    }
    case starlark_numeric_type::kBigInt: {
      const auto& idx = other.as_bigint();
      if (!idx.fits_in_int64()) {
        error_callback.add_error(error_index_out_of_range(type()));
        return status_or<int64_t>(status_code::kError);
      }
      auto iidx = idx.as_int64();
      if (idx.sign()) {
        iidx += obj_len;
      }
      if (iidx < 0 || obj_len <= iidx) {
        error_callback.add_error(error_index_out_of_range(type()));
        return status_or<int64_t>(status_code::kError);
      }
      return status_or<int64_t>(iidx);
    }
    default:
      error_callback.add_error(error_index_integer_or_slice(type(), other.type()));
      return status_or<int64_t>(status_code::kError);
  }
}

status_or<std::tuple<int64_t, int64_t, int64_t>> starlark_obj::inner_slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, int64_t len, error_fn& error_callback) const {
  /*
  This implementation does not fully folllow the same logic as Bazel and aligns better with Python.
  The discrepancies are not significant and mostly impact the representation of `range` for some edge cases.
  Eg, for `range(-3, -2, 1)[-2:-1:-3]`, Bazel returns `range(-3, -3, -3)` and Python returns `range(-4, -3, -3)`.
  These two ranges are equal, and the difference is only visible by calling `str`.
  */
  int64_t i_stride = 1;
  if (!to_int64_with_clamping_for_index_allow_none(stride, i_stride, error_callback).ok()) {
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  if (i_stride == 0) {
    error_callback.add_error(error_step_non_zero());
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  int64_t i_start = i_stride > 0 ? 0 : len - 1;
  int64_t i_end = i_stride > 0 ? len : -len - 1;
  int64_t lower = i_stride > 0 ? 0 : -1;
  int64_t upper = i_stride > 0 ? len : lower + len;
  if (!to_int64_with_clamping_for_index_allow_none(start, i_start, error_callback).ok()) {
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  if (!to_int64_with_clamping_for_index_allow_none(stop, i_end, error_callback).ok()) {
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  if (i_start < 0) {
    i_start = std::max<int64_t>(i_start + len, lower);
  } else {
    i_start = std::min<int64_t>(i_start, upper);
  }
  if (i_end < 0) {
    i_end = std::max<int64_t>(i_end + len, lower);
  } else {
    i_end = std::min<int64_t>(i_end, upper);
  }
  return status_or<std::tuple<int64_t, int64_t, int64_t>>(std::make_tuple(i_start, i_end, i_stride));
}

starlark::result::status_or<std::tuple<int64_t, int64_t, int64_t>> starlark_obj::inner_slice_range_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, int64_t original_start, int64_t original_end, int64_t original_stride, int64_t original_length, error_fn& error_callback) const {
  auto slice_result = inner_slice_range(start, stop, stride, original_length, error_callback);
  if (!slice_result.ok()) {
    return slice_result;
  }
  auto i_start = std::get<0>(*slice_result);
  auto i_end = std::get<1>(*slice_result);
  auto i_stride = std::get<2>(*slice_result);

  int64_t r_stride;
  if (ckd_mul(&r_stride, original_stride, i_stride)) {
    error_callback.add_error(error_overflow_too_many_digits());
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  int64_t tmp;
  if (ckd_mul(&tmp, original_stride, i_start)) {
    error_callback.add_error(error_overflow_too_many_digits());
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  int64_t r_start;
  if (ckd_add(&r_start, original_start, tmp)) {
    error_callback.add_error(error_overflow_too_many_digits());
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  if (ckd_mul(&tmp, original_stride, i_end)) {
    error_callback.add_error(error_overflow_too_many_digits());
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  int64_t r_end;
  if (ckd_add(&r_end, original_start, tmp)) {
    error_callback.add_error(error_overflow_too_many_digits());
    return status_or<std::tuple<int64_t, int64_t, int64_t>>(status_code::kError);
  }
  return status_or<std::tuple<int64_t, int64_t, int64_t>>(std::make_tuple(r_start, r_end, r_stride));
}

size_t starlark_hash_op::operator()(const starlark_obj* value) const {
  return value->hash();
}

bool starlark_equals_to::operator()(const starlark_obj* lhs, const starlark_obj* rhs) const {
  return lhs->equals(*rhs);
}

int64_t starlark_hash(std::span<int64_t> values) {
  constexpr uint64_t hash_prime1 = 11400714785074694791UL;
  constexpr uint64_t hash_prime2 = 14029467366897019727UL;
  constexpr uint64_t hash_prime5 = 2870177450012600261UL;

  int64_t acc = hash_prime5;
  for (const auto& element_hash : values) {
    if (element_hash == -1) {
      return -1;
    }
    acc += element_hash * hash_prime2;
    acc = std::rotl<uint64_t>(acc, 31);
    acc *= hash_prime1;
  }
  acc += values.size() ^ (hash_prime5 ^ 3527539UL);
  if (acc == -1) {
    return 1546275796;
  }
  return acc;
}

status no_named_args(const starlark_obj::named_args_t& named_args, error_fn& error_callback, std::string_view fn_name) {
  if (!named_args.empty()) {
    error_callback.add_error(error_no_keyword(fn_name));
    return error_status();
  }
  return ok_status();
}

status min_args(const starlark_obj::pos_args_t& pos_args, error_fn& error_callback, std::string_view fn_name, int expected_min) {
  if (pos_args.size() < expected_min) {
    error_callback.add_error(error_arguments_too_few(fn_name, pos_args.size(), expected_min));
    return error_status();
  }
  return ok_status();
}

status max_args(const starlark_obj::pos_args_t& pos_args, error_fn& error_callback, std::string_view fn_name, int expected_max) {
  if (pos_args.size() > expected_max) {
    error_callback.add_error(error_arguments_too_many(fn_name, pos_args.size(), expected_max));
    return error_status();
  }
  return ok_status();
}

status no_arg(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, error_fn& error_callback, std::string_view fn_name) {
  if (!no_named_args(named_args, error_callback, fn_name).ok()) {
    return error_status();
  }
  if (!pos_args.empty()) {
    error_callback.add_error(error_no_pos_args(fn_name, pos_args.size()));
    return error_status();
  }
  return ok_status();
}

status one_pos_arg(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, error_fn& error_callback, std::string_view fn_name) {
  if (!no_named_args(named_args, error_callback, fn_name).ok()) {
    return error_status();
  }
  if (pos_args.size() != 1) {
    error_callback.add_error(error_arguments_exactly_one(fn_name, pos_args.size()));
    return error_status();
  }
  return ok_status();
}

status n_pos_args(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, int pos_args_count, error_fn& error_callback, std::string_view fn_name) {
  if (!no_named_args(named_args, error_callback, fn_name).ok()) {
    return error_status();
  }
  if (pos_args.size() != pos_args_count) {
    error_callback.add_error(error_arguments_exactly(fn_name, pos_args.size(), pos_args_count));
    return error_status();
  }
  return ok_status();
}

status zero_or_one_pos_arg(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, error_fn& error_callback, std::string_view fn_name) {
  if (!no_named_args(named_args, error_callback, fn_name).ok() ||
      !max_args(pos_args, error_callback, fn_name, 1).ok()) {
    return error_status();
  }
  return ok_status();
}

status_or<int64_t> to_int64_with_clamping(const starlark_obj& iidx, error_fn& error_callback) {
  switch (iidx.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return status_or<int64_t>(iidx.as_int64());
    case starlark_numeric_type::kBigInt:
      if (!iidx.as_bigint().fits_in_int64()) {
        if (iidx.as_bigint().sign()) {
          return status_or<int64_t>(std::numeric_limits<int64_t>::min());
        } else {
          return status_or<int64_t>(std::numeric_limits<int64_t>::max());
        }
      } else {
        return status_or<int64_t>(iidx.as_bigint().as_int64());
      }
      break;
    default:
      error_callback.add_error(error_interpreted_as_integer(iidx.type()));
      return status_or<int64_t>(status_code::kError);
  }
}

status_or<int64_t> to_int64_with_clamping_for_index(const starlark_obj& iidx, error_fn& error_callback) {
  switch (iidx.numeric_type()) {
    case starlark_numeric_type::kInt64:
      return status_or<int64_t>(iidx.as_int64());
      break;
    case starlark_numeric_type::kBigInt:
      if (!iidx.as_bigint().fits_in_int64()) {
        if (iidx.as_bigint().sign()) {
          return status_or<int64_t>(std::numeric_limits<int64_t>::min());
        } else {
          return status_or<int64_t>(std::numeric_limits<int64_t>::max());
        }
      } else {
        return status_or<int64_t>(iidx.as_bigint().as_int64());
      }
      break;
    default:
      error_callback.add_error(error_index_integer_on_a_slice(iidx.type()));
      return status_or<int64_t>(status_code::kError);
  }
  return status_or<int64_t>(status_code::kError);
}

status to_int64_with_clamping_for_index_allow_none(const starlark_obj& iidx, int64_t& idx, error_fn& error_callback) {
  switch (iidx.numeric_type()) {
    case starlark_numeric_type::kInt64:
      idx = iidx.as_int64();
      break;
    case starlark_numeric_type::kBigInt:
      if (!iidx.as_bigint().fits_in_int64()) {
        if (iidx.as_bigint().sign()) {
          idx = std::numeric_limits<int64_t>::min();
        } else {
          idx = std::numeric_limits<int64_t>::max();
        }
      } else {
        idx = iidx.as_bigint().as_int64();
      }
      break;
    default:
      if (iidx.type() == starlark_types::none_t) {
        return ok_status();
      }
      error_callback.add_error(error_index_integer_on_a_slice(iidx.type()));
      return error_status();
  }
  return ok_status();
}

int64_t calculate_len(int64_t start, int64_t end, int64_t step) {
  assert(step != 0);
  if (step > 0 && start < end) {
    return (end - 1 - start) / step + 1;
  } else if (step < 0 && start > end) {
    return (start - 1 - end) / -step + 1;
  } else {
    return 0;
  }
}

}  // namespace runtime
}  // namespace starlark


