// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_tuple.hpp"

#include <stdckdint.h>

#include <algorithm>
#include <bit>
#include <string>
#include <vector>

#include "errors/runtime_error_messages.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::error_messages::error_v2_max_sequence_length;
using ::starlark::error_messages::error_v2_no_concat;
using ::starlark::error_messages::error_v2_no_multiply_sequence;
using ::starlark::error_messages::error_v2_unpack_too_few;
using ::starlark::error_messages::error_v2_unpack_too_many;

namespace starlark {
namespace runtime {

const std::vector<starlark_obj*>& inspect_tuple(const starlark_tuple& tuple) {
  return tuple.values;
}

starlark_tuple::starlark_tuple(std::size_t reserve_size) : starlark_obj(object_kind::kTuple) {
  values.reserve(reserve_size);
}


namespace {

starlark_obj* plus_op(const starlark_tuple& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  if (!same_starlark_type(other.kind(), this_obj.kind())) {
    error_callback.add_error(error_v2_no_concat(this_obj.type(), other.type(), this_obj.type()));
    return nullptr;
  }
  const starlark_tuple& t_other = static_cast<const starlark_tuple&>(other);
  const auto& this_values = inspect_tuple(this_obj);
  const auto& other_values = inspect_tuple(t_other);
  std::size_t expected_size;
  if (ckd_add(&expected_size, this_values.size(), other_values.size()) ||
      expected_size > ctx.options().max_sequence_size) {
    error_callback.add_error(error_v2_max_sequence_length(ctx.options().max_sequence_size));
    return nullptr;
  }
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), expected_size);
  for (auto& key : this_values) {
    result->add(key);
  }
  for (auto& key : other_values) {
    result->add(key);
  }
  return result;
}

starlark_obj* star_op(const starlark_tuple& this_obj, const starlark_obj& other, std::string_view op, context& ctx, error_fn& error_callback) {
  const auto& this_values = inspect_tuple(this_obj);
  switch (other.kind()) {
    case object_kind::kInt: {
      if (this_values.empty()) {
        return Arena::Create<starlark_tuple>(&ctx.arena(), 0);
      }
      auto value = other.as_int64();
      if (value <= 0) {
        return Arena::Create<starlark_tuple>(&ctx.arena(), 0);
      }
      std::size_t expected_size;
      if (ckd_mul(&expected_size, this_values.size(), value) ||
          expected_size > ctx.options().max_sequence_size) {
        error_callback.add_error(error_v2_max_sequence_length(ctx.options().max_sequence_size));
        return nullptr;
      }
      auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), expected_size);
      for (int64_t i = 0; i < value; ++i) {
        for (auto& key : this_values) {
          result->add(key);
        }
      }
      return result;
    }
    case object_kind::kBigInt: {
      if (this_values.empty()) {
        return Arena::Create<starlark_tuple>(&ctx.arena(), 0);
      }
      const auto& value = other.as_bigint();
      if (value <= number::zero()) {
        return Arena::Create<starlark_tuple>(&ctx.arena(), 0);
      }
      if (value.bit_size() >= 63) {
        error_callback.add_error(error_v2_max_sequence_length(ctx.options().max_sequence_size));
        return nullptr;
      }
      int64_t int_value = value.at(0);
      std::size_t expected_size;
      if (ckd_mul(&expected_size, this_values.size(), int_value) ||
          expected_size > ctx.options().max_sequence_size) {
        error_callback.add_error(error_v2_max_sequence_length(ctx.options().max_sequence_size));
        return nullptr;
      }
      auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), expected_size);
      for (int64_t i = 0; i < int_value; ++i) {
        for (auto& key : this_values) {
          result->add(key);
        }
      }
      return result;
    }
    default:
      error_callback.add_error(error_v2_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

}  // namespace

starlark_obj* starlark_tuple::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return plus_op(*this, other, "+", ctx, error_callback);
}

starlark_obj* starlark_tuple::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return plus_op(*this, other, "+=", ctx, error_callback);
}

starlark_obj* starlark_tuple::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  return star_op(*this, other, "*", ctx, error_callback);
}

starlark_obj* starlark_tuple::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  return star_op(*this, other, "*=", ctx, error_callback);
}

int64_t starlark_tuple::unsafe_len() const {
  return values.size();
}

bool starlark_tuple::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values.size() == 0) {
        print.append("()");
        return false;
      }
      print.append("(");
      printer_action new_action = printer_action::kPrintFinal;
      for (auto it = values.rbegin(); it != values.rend(); ++it) {
        print.add_task(printer::pending_task{
          .obj = this,
          .action = new_action,
        });
        print.add_task(printer::pending_task{
          .obj = *it,
          .action = printer_action::kPrintTop,
        });
        new_action = printer_action::kPrintElementSeparator;
      }
      return true;
    }
    case printer_action::kPrintElementSeparator:
    case printer_action::kPrintInElementSeparator:
      print.append(", ");
      return true;
    case printer_action::kPrintFinal:
      if (values.size() == 1) {
        print.append(",)");
      } else {
        print.append(")");
      }
      return false;
    case printer_action::kPrintRecursion:
    default:
      print.append("(...)");
      return false;
  }
}

bool starlark_tuple::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (!same_starlark_type(kind(), other->kind())) {
    return false;
  }
  const starlark_tuple* t_other = reinterpret_cast<const starlark_tuple*>(other);
  if (values.size() != t_other->values.size()) {
    return false;
  }
  for (int i = 0; i < values.size(); ++i) {
    comp.add_task(equals_comparator::pending_task{
      .lhs = values[i],
      .rhs = t_other->values[i],
    });
  }
  return true;
}

void starlark_tuple::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (!same_starlark_type(other->kind(), kind())) {
    starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
    return;
  }
  const auto* t_other = static_cast<const starlark_tuple*>(other);
  if (values.size() != t_other->values.size()) {
    comp.add_task(values.size() > t_other->values.size() ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
  for (int i = std::min(values.size(), t_other->values.size()) - 1; i >= 0; --i) {
    comp.add_task(values[i], t_other->values[i], true);
  }
}

bool starlark_tuple::truthy() const {
  return !values.empty();
}

void starlark_tuple::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, context& ctx, error_fn& error_callback) {
  if (number_of_elements != values.size()) {
    if (values.size() < number_of_elements) {
      error_callback.add_error(error_v2_unpack_too_few(values.size(), number_of_elements));
    } else {
      error_callback.add_error(error_v2_unpack_too_many(values.size(), number_of_elements));
    }
    return;
  }

  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    consumer.push_back(*it);
  }
}

bool starlark_tuple::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  for (const auto& element : values) {
    if (other.equals(*element)) {
      return true;
    }
  }
  return false;
}

starlark_iterator* starlark_tuple::get_iterator(bool produce_error, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_tuple_iterator>(&ctx.arena(), this);
}

starlark_obj* starlark_tuple::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  auto idx = inner_index(other, values.size(), error_callback);
  if (!idx.ok()) {
    return nullptr;
  }
  return values[*idx];
}

starlark_obj* starlark_tuple::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
  auto slice_result = inner_slice_range(start, stop, stride, values.size(), error_callback);
  if (!slice_result.ok()) {
    return nullptr;
  }
  auto i_start = std::get<0>(*slice_result);
  auto i_end = std::get<1>(*slice_result);
  auto i_stride = std::get<2>(*slice_result);
  auto* result = Arena::Create<starlark_tuple>(&ctx.arena(), calculate_len(i_start, i_end, i_stride));
  if (i_stride > 0) {
    for (auto i = i_start; i < i_end; i += i_stride) {
      result->values.push_back(values[i]);
    }
  } else {
    for (auto i = i_start; i > i_end; i += i_stride) {
      result->values.push_back(values[i]);
    }
  }
  return result;
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_tuple::inner_hash() const {
  return std::span(values.begin(), values.end());
}

void starlark_tuple::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto* element : values) {
    to_freeze.push_back(element);
  }
}

starlark_tuple& starlark_tuple::add(starlark_obj* element) {
  values.push_back(element);
  return *this;
}

starlark_tuple::starlark_tuple_iterator::starlark_tuple_iterator(starlark_tuple* tuple) : tuple(tuple), it(tuple->values.begin()) {}

bool starlark_tuple::starlark_tuple_iterator::has_next() const {
  return it != tuple->values.end();
}

starlark_obj* starlark_tuple::starlark_tuple_iterator::next() {
  return *it++;
}

void starlark_tuple::starlark_tuple_iterator::end_iterator() {}

std::size_t starlark_tuple::size() const {
  return values.size();
}

const starlark_obj* starlark_tuple::at(std::size_t pos) const {
  return values[pos];
}

}  // namespace runtime
}  // namespace starlark


