// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_list.hpp"

#include <algorithm>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <vector>

#include "runtime/error_messages.hpp"
#include "runtime/options.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_numeric.hpp"
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

starlark_obj* starlark_list::binary_plus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  if (other.type() != type()) {
    error_callback.add_error(error_no_concat(type(), other.type(), type()));
    return nullptr;
  }
  const starlark_list& l_other = static_cast<const starlark_list&>(other);
  // TODO(lmirelmann): Check the result size.
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), values.size() + l_other.values.size());
  // TODO(lmirelmann): It should be possible to insert the entire thing using one call to `std::vector::insert`, but
  // this would slightly break the fact that `add` is the only one adding elements.
  for (auto& key : values) {
    result->append(key, error_callback);
  }
  for (auto& key : l_other.values) {
    result->append(key, error_callback);
  }
  return result;
}

starlark_obj* starlark_list::binary_star(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (values.empty()) {
        return Arena::Create<starlark_list>(&ctx.arena(), 0);
      }
      auto value = other.as_int64();
      if (value <= 0) {
        return Arena::Create<starlark_list>(&ctx.arena(), 0);
      }
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_list>(&ctx.arena(), value * values.size());
      for (int64_t i = 0; i < value; ++i) {
        for (auto& key : values) {
          result->append(key, error_callback);
        }
      }
      return result;
    }
    case starlark_numeric_type::kBigInt: {
      if (values.empty()) {
        return Arena::Create<starlark_list>(&ctx.arena(), 0);
      }
      const auto& value = other.as_bigint();
      if (value <= number::zero()) {
        return Arena::Create<starlark_list>(&ctx.arena(), 0);
      }
      if (value.bit_size() >= 63) {
        error_callback.add_error(error_max_sequence_length(max_sequence_size()));
        return nullptr;
      }
      int64_t int_value = value.at(0);
      // TODO(lmirelmann): Check whether the size will be over the maximum allowed.
      auto* result = Arena::Create<starlark_list>(&ctx.arena(), int_value * values.size());
      for (int64_t i = 0; i < int_value; ++i) {
        for (auto& key : values) {
          result->append(key, error_callback);
        }
      }
      return result;
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

starlark_obj* starlark_list::plus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  if (other.type() != type()) {
    error_callback.add_error(error_no_concat(type(), other.type(), type()));
    return nullptr;
  }
  if (!can_modify("append", error_callback)) {
    return nullptr;
  }
  const starlark_list& l_other = static_cast<const starlark_list&>(other);
  // TODO(lmirelmann): Check the result size.
  // This needs to be able to handle the case `a += a`
  for (int i = 0, e = l_other.values.size(); i < e; ++i) {
    append(l_other.values[i], error_callback);
  }
  return this;
}

starlark_obj* starlark_list::star_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  switch (other.numeric_type()) {
    case starlark_numeric_type::kInt64: {
      if (!can_modify("append", error_callback)) {
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
          append(values[j], error_callback);
        }
      }
      return this;
    }
    case starlark_numeric_type::kBigInt: {
      if (!can_modify("append", error_callback)) {
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
          append(values[j], error_callback);
        }
      }
      return this;
    }
    default:
      error_callback.add_error(error_no_multiply_sequence(other.type()));
      return nullptr;
  }
}

starlark_iterator* starlark_list::get_iterator(bool produce_error, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_list_iterator>(&ctx.arena(), this);
}

starlark_obj* starlark_list::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  auto idx = inner_index(other, values.size(), error_callback);
  if (idx < 0) {
    return nullptr;
  }
  return values[idx];
}

void starlark_list::index_assign(const starlark_obj& idx, starlark_obj& element, error_fn& error_callback) {
  if (!can_modify("update", error_callback)) {
    return;
  }
  auto iidx = inner_index(idx, values.size(), error_callback);
  if (iidx < 0) {
    return;
  }
  values[iidx] = &element;
}

starlark_obj* starlark_list::slice_range(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, context& ctx, error_fn& error_callback) const {
  auto [i_start, i_end, i_stride, valid] = inner_slice_range(start, stop, stride, values.size(), error_callback);
  if (!valid) {
    return nullptr;
  }

  // TODO(lmirelmann): Pre-calculate the size.
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), 0);
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

bool starlark_list::append(starlark_obj* element, error_fn& error_callback) {
  if (!can_modify("append", error_callback)) {
    return false;
  }
  // TODO(lmirelmann): Check that the number of elements does not go over the maximum.
  values.push_back(element);
  return true;
}

bool starlark_list::clear(error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return false;
  }
  values.clear();
  return true;
}

bool starlark_list::extend(starlark_obj* other, context& ctx, error_fn& error_callback) {
  if (!can_modify("append", error_callback)) {
    return false;
  }
  if (other->type() == type()) {
    starlark_list* lother = static_cast<starlark_list*>(other);
    for (decltype(values)::size_type i = 0, end = lother->values.size(); i < end; ++i) {
      values.push_back(lother->values[i]);
    }
  } else {
    auto* it = other->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return false;
    }
    while (it->has_next()) {
      values.push_back(it->next());
    }
    it->end_iterator();
  }
  return true;
}

starlark_obj* starlark_list::index(starlark_obj* element, int64_t start, int64_t end, context& ctx, error_fn& error_callback) const {
  if (start < 0) {
    start = std::max<int64_t>(start + values.size(), 0);
  } else {
    start = std::min<int64_t>(start, values.size());
  }
  if (end < 0) {
    end = std::max<int64_t>(end + values.size(), 0);
  } else {
    end = std::min<int64_t>(end, values.size());
  }
  for (auto i = start; i < end; ++i) {
    if (values[i]->equals(*element)) {
      return create_integer(i, ctx);
    }
  }
  error_callback.add_error(error_item_not_in_collection(type(), "index"));
  return nullptr;
}

bool starlark_list::insert(starlark_obj* element, int64_t pos, error_fn& error_callback) {
  if (!can_modify("append", error_callback)) {
    return false;
  }
  if (pos < 0) {
    pos = std::max<int64_t>(pos + values.size(), 0);
  } else {
    pos = std::min<int64_t>(pos, values.size());
  }
  values.insert(values.begin() + pos, element);
  return true;
}

starlark_obj* starlark_list::pop(int64_t idx, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return nullptr;
  }
  if (idx < 0) {
    idx += values.size();
  }
  if (idx < 0 || idx >= values.size()) {
    error_callback.add_error(error_index_out_of_range("pop"));
    return nullptr;
  }
  auto* result = values[idx];
  values.erase(values.begin() + idx);
  return result;
}

bool starlark_list::remove(starlark_obj* element, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return false;
  }
  for (auto it = values.begin(); it != values.end(); ++it) {
    if ((*it)->equals(*element)) {
      values.erase(it);
      return true;
    }
  }
  error_callback.add_error(error_item_not_in_collection(type(), "remove"));
  return false;
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

bool starlark_list::can_modify(std::string_view op, error_fn& error_callback) const {
  if (iterators_count) {
    error_callback.add_error(error_op_in_loop(type(), op));
    return false;
  }
  if (freezed) {
    error_callback.add_error(error_mutate_frozen_value(type()));
    return false;
  }
  return true;
}

starlark_obj* starlark_list_fn_append(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "list.append")) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::list_t);
  if (!static_cast<starlark_list*>(this_obj)->append(pos_args.front(), error_callback)) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_list_fn_clear(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "list.clear")) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::list_t);
  if (!static_cast<starlark_list*>(this_obj)->clear(error_callback)) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_list_fn_extend(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "list.extend")) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::list_t);
  if (!static_cast<starlark_list*>(this_obj)->extend(pos_args.front(), ctx, error_callback)) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_list_fn_index(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "list.index") ||
      !min_args(pos_args, error_callback, "index", 1) ||
      !max_args(pos_args, error_callback, "index", 3)) {
    return nullptr;
  }
  int64_t start = 0;
  int64_t end = -1;
  if (pos_args.size() >= 2) {
    if (!to_int64_with_clamping_for_index_allow_none(*pos_args[1], start, error_callback)) {
      return nullptr;
    }
    if (pos_args.size() >= 3) {
      if (!to_int64_with_clamping_for_index_allow_none(*pos_args[2], end, error_callback)) {
        return nullptr;
      }
    }
  }
  return static_cast<starlark_list*>(this_obj)->index(pos_args.front(), start, end, ctx, error_callback);
}

starlark_obj* starlark_list_fn_insert(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!n_pos_args(pos_args, named_args, 2, error_callback, "list.insert")) {
    return nullptr;
  }
  int64_t idx;
  if (!to_int64_with_clamping_for_index(*pos_args.front(), idx, error_callback)) {
    return nullptr;
  }
  if (!static_cast<starlark_list*>(this_obj)->insert(pos_args[1], idx, error_callback)) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_list_fn_pop(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!zero_or_one_pos_arg(pos_args, named_args, error_callback, "list.pop")) {
     return nullptr;
  }
  int64_t idx = -1;
  if (pos_args.size() >= 1 &&
      !to_int64_with_clamping_for_index(*pos_args.front(), idx, error_callback)) {
    return nullptr;
  }
  return static_cast<starlark_list*>(this_obj)->pop(idx, error_callback);
}

starlark_obj* starlark_list_fn_remove(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "list.remove")) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::list_t);
  if (!static_cast<starlark_list*>(this_obj)->remove(pos_args.front(), error_callback)) {
    return nullptr;
  }
  return ctx.none_value();
}

}  // namespace runtime
}  // namespace starlark


