// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_list.hpp"

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "runtime/error_messages.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

starlark_obj* starlark_list_fn_append(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_list_fn_clear(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_list_fn_extend(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_list_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_list_fn_insert(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_list_fn_pop(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_list_fn_remove(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_list::method_refs() {
  static const std::map<std::string, starlark_obj::fn*, std::less<>>* result =
    new std::map<std::string, starlark_obj::fn*, std::less<>>{
      {"append", starlark_list_fn_append},
      {"clear", starlark_list_fn_clear},
      {"extend", starlark_list_fn_extend},
      {"index", starlark_list_fn_index},
      {"insert", starlark_list_fn_insert},
      {"pop", starlark_list_fn_pop},
      {"remove", starlark_list_fn_remove},
    };

  return *result;
}

const std::vector<std::string>& starlark_list::attributes() {
  static const std::vector<std::string>* result =
    new std::vector<std::string>(([]() {
      std::vector<std::string> result;
      result.reserve(method_refs().size());
      for (const auto& [k, v] : method_refs()) {
        result.push_back(k);
      }
      return result;
    })());

  return *result;
}

starlark_list::starlark_list(std::size_t reserve_size) {
  values.reserve(reserve_size);
}

std::string_view starlark_list::type() const {
  return starlark_types::list_t;
}

const std::vector<std::string>& starlark_list::dir() const {
  return attributes();
}

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_list::methods_meta() const {
  return method_refs();
}

int64_t starlark_list::len(bool produce_error, error_fn& error_callback) const {
  return values.size();
}

bool starlark_list::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values.size() == 0) {
        print.append("[]");
        return false;
      }
      print.append("[");
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
      print.append("]");
      return false;
    case printer_action::kPrintRecursion:
      print.append("[...]");
      return false;
  }
}

bool starlark_list::truthy() const {
  return !values.empty();
}

void starlark_list::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn& error_callback) {
  if (number_of_elements != values.size()) {
    if (values.size() < number_of_elements) {
      error_callback.add_error(error_unpack_too_few(values.size(), number_of_elements));
    } else {
      error_callback.add_error(error_unpack_too_many(values.size(), number_of_elements));
    }
    return;
  }
  for (auto it = values.rbegin(); it != values.rend(); ++it) {
    consumer.push_back(*it);
  }
}

bool starlark_list::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  for (const auto& element : values) {
    if (other.equals(*element)) {
      return true;
    }
  }
  return false;
}

starlark_obj* starlark_list::binary_plus(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  if (other.type() != type()) {
    error_callback.add_error(error_no_concat(type(), other.type(), type()));
    return nullptr;
  }
  const starlark_list& l_other = static_cast<const starlark_list&>(other);
  // TODO(lmirelmann): Check the result size.
  auto* result = Arena::Create<starlark_list>(&arena, values.size() + l_other.values.size());
  // TODO(lmirelmann): It should be possible to insert the entire thing using one call to `std::vector::insert`, but
  // this would slightly break the fact that `add` is the only one adding elements.
  for (auto& key : values) {
    result->add(key, error_callback);
  }
  for (auto& key : l_other.values) {
    result->add(key, error_callback);
  }
  return result;
}

starlark_obj* starlark_list::binary_star(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (values.empty()) {
        return Arena::Create<starlark_list>(&arena, 0);
      }
      auto value = other.as_int64();
      if (value <= 0) {
        return Arena::Create<starlark_list>(&arena, 0);
      }
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_list>(&arena, value * values.size());
      for (int64_t i = 0; i < value; ++i) {
        for (auto& key : values) {
          result->add(key, error_callback);
        }
      }
      return result;
    }
    case starlark_numeric_type::kBigInt: {
      if (values.empty()) {
        return Arena::Create<starlark_list>(&arena, 0);
      }
      const auto& value = other.as_bigint();
      if (value <= number::zero()) {
        return Arena::Create<starlark_list>(&arena, 0);
      }
      if (value.bit_size() >= 63) {
        error_callback.add_error(error_max_sequence_length(max_sequence_size()));
        return nullptr;
      }
      int64_t int_value = value.at(0);
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_list>(&arena, int_value * values.size());
      for (int64_t i = 0; i < int_value; ++i) {
        for (auto& key : values) {
          result->add(key, error_callback);
        }
      }
      return result;
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

starlark_obj* starlark_list::plus_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) {
  if (other.type() != type()) {
    error_callback.add_error(error_no_concat(type(), other.type(), type()));
    return nullptr;
  }
  if (!can_modify(error_callback)) {
    return nullptr;
  }
  const starlark_list& l_other = static_cast<const starlark_list&>(other);
  // TODO(lmirelmann): Check the result size.
  // This needs to be able to handle the case `a += a`
  for (int i = 0, e = l_other.values.size(); i < e; ++i) {
    add(l_other.values[i], error_callback);
  }
  return this;
}

starlark_obj* starlark_list::star_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (!can_modify(error_callback)) {
        return nullptr;
      }
      if (values.empty()) {
        return this;
      }
      auto value = other.as_int64();
      if (value <= 0) {
        values.clear();
        return this;
      }
      auto original_size = values.size();
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      for (int64_t i = 1; i < value; ++i) {
        for (int j = 0; j < original_size; ++j) {
          add(values[j], error_callback);
        }
      }
      return this;
    }
    case starlark_numeric_type::kBigInt: {
      if (!can_modify(error_callback)) {
        return nullptr;
      }
      if (values.empty()) {
        return this;
      }
      const auto& value = other.as_bigint();
      if (value <= number::zero()) {
        values.clear();
        return this;
      }
      if (value.bit_size() >= 63) {
        error_callback.add_error(error_max_sequence_length(max_sequence_size()));
        return nullptr;
      }
      int64_t int_value = value.at(0);
      auto original_size = values.size();
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      for (int64_t i = 1; i < int_value; ++i) {
        for (int j = 0; j < original_size; ++j) {
          add(values[j], error_callback);
        }
      }
      return this;
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

starlark_iterator* starlark_list::get_iterator(bool produce_error, Arena& arena, error_fn& error_callback) {
  return Arena::Create<starlark_list_iterator>(&arena, this);
}

starlark_obj* starlark_list::index(const starlark_obj& other, Arena& arena, error_fn& error_callback) const {
  auto idx = inner_index(other, values.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return values[idx];
}

void starlark_list::index_assign(const starlark_obj& idx, starlark_obj& element, error_fn& error_callback) {
  if (!can_modify(error_callback)) {
    return;
  }
  auto iidx = inner_index(idx, values.size(), error_callback);
  if (iidx < 0) {
    return;
  }
  values[iidx] = &element;
}

bool starlark_list::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_list* l_other = reinterpret_cast<const starlark_list*>(other);
  if (values.size() != l_other->values.size()) {
    return false;
  }
  for (int i = 0; i < values.size(); ++i) {
    comp.add_task(equals_comparator::pending_task{
      .lhs = values[i],
      .rhs = l_other->values[i],
    });
  }
  return true;
}

void starlark_list::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const {
  if (other->type() != type()) {
    starlark_obj::inner_cmp(comp, other, op, error_callback);
    return;
  }
  const auto* l_other = static_cast<const starlark_list*>(other);
  if (values.size() != l_other->values.size()) {
    comp.add_task(values.size() > l_other->values.size() ? order_comparator::pending_task_type::kGreaterThan : order_comparator::pending_task_type::kLessThan);
  }
  for (int i = std::min(values.size(), l_other->values.size()) - 1; i >= 0; --i) {
    comp.add_task(values[i], l_other->values[i]);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_list::inner_hash() const {
  // My current understanding is that this is the right behavior.
  return -1;
}

void starlark_list::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto* element : values) {
    to_freeze.push_back(element);
  }
}

void starlark_list::add(starlark_obj* element, error_fn& error_callback) {
  if (!can_modify(error_callback)) {
    return;
  }
  // TODO(lmirelmann): Check that this does not go over the maximum number of elements.
  values.push_back(element);
}

starlark_list::starlark_list_iterator::starlark_list_iterator(starlark_list* list) : list(list), it(list->values.begin()) {
  list->iterators_count++;
}

bool starlark_list::starlark_list_iterator::has_next() const {
  return it != list->values.end();
}

starlark_obj* starlark_list::starlark_list_iterator::next() {
  return *it++;
}

void starlark_list::starlark_list_iterator::end_iterator() {
  list->iterators_count--;
}

bool starlark_list::can_modify(error_fn& error_callback) const {
  if (iterators_count) {
    error_callback.add_error(error_append_in_loop(type()));
    return false;
  }
  if (freezed) {
    error_callback.add_error(error_mutate_frozen_value(type()));
    return false;
  }
  return true;
}

starlark_obj* starlark_list_fn_append(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_list_fn_clear(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_list_fn_extend(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_list_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_list_fn_insert(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_list_fn_pop(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

starlark_obj* starlark_list_fn_remove(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // TODO(lmirelmann): Implement.
  error_callback.add_error("Unimplemented");
  return nullptr;
}

}  // namespace runtime
}  // namespace starlark


