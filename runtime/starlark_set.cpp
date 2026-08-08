// Copyright 2025-2026 Lucas Mirelmann

#include "runtime/starlark_set.hpp"

#include <functional>
#include <iterator>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "errors/runtime_error_messages.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::error_messages::error_dictionary_key_not_found;
using ::starlark::error_messages::error_mutate_frozen_value;
using ::starlark::error_messages::error_op_in_loop;
using ::starlark::error_messages::error_v2_empty_set;
using ::starlark::error_messages::error_v2_unhashable_value;
using ::starlark::error_messages::error_v2_unpack_too_few;
using ::starlark::error_messages::error_v2_unpack_too_many;
using ::starlark::result::error_status;
using ::starlark::result::ok_status;
using ::starlark::result::status;
using ::starlark::result::status_code;
using ::starlark::result::status_or;

namespace starlark {
namespace runtime {

starlark_obj* starlark_set_fn_add(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_clear(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_difference(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_difference_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_discard(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_intersection(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_intersection_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_isdisjoint(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_issubset(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_issuperset(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_pop(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_remove(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_symmetric_difference(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_symmetric_difference_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_union(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_set_fn_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_set::method_refs() {
  static const std::map<std::string, starlark_obj::fn*, std::less<>>* result =
    new std::map<std::string, starlark_obj::fn*, std::less<>>{
      {"add", starlark_set_fn_add},
      {"clear", starlark_set_fn_clear},
      {"difference", starlark_set_fn_difference},
      {"difference_update", starlark_set_fn_difference_update},
      {"discard", starlark_set_fn_discard},
      {"intersection", starlark_set_fn_intersection},
      {"intersection_update", starlark_set_fn_intersection_update},
      {"isdisjoint", starlark_set_fn_isdisjoint},
      {"issubset", starlark_set_fn_issubset},
      {"issuperset", starlark_set_fn_issuperset},
      {"pop", starlark_set_fn_pop},
      {"remove", starlark_set_fn_remove},
      {"symmetric_difference", starlark_set_fn_symmetric_difference},
      {"symmetric_difference_update", starlark_set_fn_symmetric_difference_update},
      {"union", starlark_set_fn_union},
      {"update", starlark_set_fn_update},
    };

  return *result;
}

const std::vector<std::string>& starlark_set::attributes() {
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

starlark_set::starlark_set() : iterators_count(0) {}

std::string_view starlark_set::type() const {
  return starlark_types::set_t;
}

const std::vector<std::string>& starlark_set::dir() const {
  return attributes();
}

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_set::methods_meta() const {
  return method_refs();
}

void starlark_set::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, context& ctx, error_fn& error_callback) {
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

int64_t starlark_set::len(bool produce_error, error_fn& error_callback) const {
  return values.size();
}

bool starlark_set::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values.size() == 0) {
        print.append("set()");
        return false;
      }
      print.append("set([");
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
      print.append("])");
      return false;
    case printer_action::kPrintRecursion:
      // This is only possible with some extension like `struct` that is hashable even when its elements are not.
      print.append("set([...])");
      return false;
  }
}

bool starlark_set::truthy() const {
  return !values.empty();
}

bool starlark_set::contains(starlark_obj* obj) const {
  return values.contains(obj);
}

bool starlark_set::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  if (other.hash() == -1) {
    error_callback.add_error(error_v2_unhashable_value(type(), other.type()));
    return false;
  }
  // The const_cast is needed as there is no conversion from `const starlark_obj *const` to `starlark_obj *const`
  return values.contains(&const_cast<starlark_obj&>(other));
}

starlark_obj* starlark_set::binary_and(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_and(other, ctx, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : values) {
    if (auto it = s_other.values.find(key); it != s_other.values.end()) {
      result->add(*it, error_callback);
    }
  }
  return result;
}

starlark_obj* starlark_set::binary_pipe(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_pipe(other, ctx, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  for (auto& key : values) {
    result->add(key, error_callback);
  }
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : s_other.values) {
    result->add(key, error_callback);
  }
  return result;
}

starlark_obj* starlark_set::binary_hat(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_hat(other, ctx, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : values) {
    if (!s_other.values.contains(key)) {
      result->add(key, error_callback);
    }
  }
  for (auto& key : s_other.values) {
    if (!values.contains(key)) {
      result->add(key, error_callback);
    }
  }
  return result;
}

starlark_obj* starlark_set::binary_minus(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_minus(other, ctx, error_callback);
  }
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : values) {
    if (!s_other.values.contains(key)) {
      result->add(key, error_callback);
    }
  }
  return result;
}

starlark_obj* starlark_set::minus_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return nullptr;
  }
  if (other.type() != type()) {
    return starlark_obj::minus_equals_assign(other, ctx, error_callback);
  }
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : set_t(s_other.values)) {
    values.erase(key);
  }
  return this;
}

starlark_obj* starlark_set::ampersand_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return nullptr;
  }
  if (other.type() != type()) {
    return starlark_obj::ampersand_equals_assign(other, ctx, error_callback);
  }
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (const auto& value : set_t(values)) {
    if (!s_other.values.contains(value)) {
      values.erase(value);
    }
  }
  return this;
}

starlark_obj* starlark_set::pipe_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  if (!can_modify("append", error_callback)) {
    return nullptr;
  }
  if (other.type() != type()) {
    return starlark_obj::pipe_equals_assign(other, ctx, error_callback);
  }
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (auto& key : s_other.values) {
    values.insert(key);
  }
  return this;
}

starlark_obj* starlark_set::hat_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  if (!can_modify("update", error_callback)) {
    return nullptr;
  }
  if (other.type() != type()) {
    return starlark_obj::hat_equals_assign(other, ctx, error_callback);
  }
  const starlark_set& s_other = static_cast<const starlark_set&>(other);
  for (const auto& value : set_t(s_other.values)) {
    if (values.contains(value)) {
      values.erase(value);
    } else {
      values.insert(value);
    }
  }
  return this;
}

starlark_iterator* starlark_set::get_iterator(bool produce_error, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_set_iterator>(&ctx.arena(), this);
}

status_or<bool> starlark_set::add(starlark_obj* element, error_fn& error_callback) {
  if (!can_modify("append", error_callback)) {
    return status_or<bool>(status_code::kRuntimeError);
  }
  if (element->hash() == -1) {
    error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
    return status_or<bool>(status_code::kRuntimeError);
  }
  return status_or<bool>(values.insert(element).second);
}

status starlark_set::clear(error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return error_status();
  }
  values.clear();
  return ok_status();
}

starlark_obj* starlark_set::difference(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback) const {
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  for (auto* value : values) {
    result->values.insert(value);
  }
  for (auto* other : others) {
    auto* it = other->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return nullptr;
    }
    while (it->has_next()) {
      if (!result->discard(it->next(), error_callback).ok()) {
        it->end_iterator();
        return nullptr;
      }
    }
    it->end_iterator();
  }
  return result;
}

status starlark_set::difference_update(std::vector<starlark_obj*> others, context& ctx, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return error_status();
  }
  for (auto* other : others) {
    if (other == this) {
      values.clear();
      continue;
    }
    auto* it = other->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return error_status();
    }
    while (it->has_next()) {
      if (!discard(it->next(), error_callback).ok()) {
        it->end_iterator();
        return error_status();
      }
    }
    it->end_iterator();
  }
  return ok_status();
}

status starlark_set::discard(starlark_obj* element, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return error_status();
  }
  if (element->hash() == -1) {
    error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
    return error_status();
  }
  values.erase(element);
  return ok_status();
}

starlark_obj* starlark_set::intersection(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback) const {
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  for (auto* value : values) {
    result->values.insert(value);
  }
  for (auto* other : others) {
    if (other == this) {
      continue;
    }
    auto* it = other->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return nullptr;
    }
    set_t other_as_set;
    while (it->has_next()) {
      auto* element = it->next();
      if (element->hash() == -1) {
        it->end_iterator();
        error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
        return nullptr;
      }
      if (result->values.contains(element)) {
        other_as_set.insert(element);
      }
    }
    it->end_iterator();
    for (const auto& value : result->values) {
      if (other_as_set.contains(value)) {
        other_as_set.erase(value);
      } else {
        other_as_set.insert(value);
      }
    }
    for (const auto& value : other_as_set) {
      result->values.erase(value);
    }
  }
  return result;
}

status starlark_set::intersection_update(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return error_status();
  }
  for (auto* other : others) {
    if (other == this) {
      continue;
    }
    auto* it = other->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return error_status();
    }
    set_t other_as_set;
    while (it->has_next()) {
      auto* element = it->next();
      if (element->hash() == -1) {
        it->end_iterator();
        error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
        return error_status();
      }
      if (values.contains(element)) {
        other_as_set.insert(element);
      }
    }
    it->end_iterator();
    // There is a lot of moving around to avoid a copy.
    // After this step, `other_as_set` will contain the elements that should be removed from `values`.
    for (const auto& value : values) {
      if (other_as_set.contains(value)) {
        other_as_set.erase(value);
      } else {
        other_as_set.insert(value);
      }
    }
    for (const auto& value : other_as_set) {
      values.erase(value);
    }
  }
  return ok_status();
}

status_or<bool> starlark_set::isdisjoint(starlark_obj* other, context& ctx, error_fn& error_callback) const {
  auto* it = other->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return status_or<bool>(status_code::kRuntimeError);
  }
  bool result = true;
  while (it->has_next()) {
    auto* element = it->next();
    if (element->hash() == -1) {
      it->end_iterator();
      error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
      return status_or<bool>(status_code::kRuntimeError);
    }
    if (values.contains(element)) {
      // Do not terminate early as the spec mandates that we have to check that every element is hashable.
      result = false;
    }
  }
  it->end_iterator();
  return status_or<bool>(result);
}

status_or<bool> starlark_set::issubset(starlark_obj* other, context& ctx, error_fn& error_callback) const {
  auto* it = other->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return status_or<bool>(status_code::kRuntimeError);
  }
  set_t other_as_set;
  while (it->has_next()) {
    auto* element = it->next();
    if (element->hash() == -1) {
      it->end_iterator();
      error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
      return status_or<bool>(status_code::kRuntimeError);
    }
    if (values.contains(element)) {
      other_as_set.insert(element);
    }
  }
  it->end_iterator();
  return status_or<bool>(values.size() == other_as_set.size());
}

status_or<bool> starlark_set::issuperset(starlark_obj* other, context& ctx, error_fn& error_callback) const {
  auto* it = other->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return status_or<bool>(status_code::kRuntimeError);
  }
  bool result = true;
  while (it->has_next()) {
    auto* element = it->next();
    if (element->hash() == -1) {
      it->end_iterator();
      error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
      return status_or<bool>(status_code::kRuntimeError);
    }
    if (!values.contains(element)) {
      // Do not terminate early as the spec mandates that we have to check that every element is hashable.
      result = false;
    }
  }
  it->end_iterator();
  return status_or<bool>(result);
}

starlark_obj* starlark_set::pop(error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return nullptr;
  }
  if (values.empty()) {
    error_callback.add_error(error_v2_empty_set("pop"));
    return nullptr;
  }
  auto* result = *values.begin();
  values.erase(result);
  return result;
}

status starlark_set::remove(starlark_obj* element, error_fn& error_callback) {
  if (!can_modify("delete", error_callback)) {
    return error_status();
  }
  if (element->hash() == -1) {
    error_callback.add_error(error_v2_unhashable_value(type(), element->type()));
    return error_status();
  }
  if (values.erase(element) == 0) {
    error_callback.add_error(error_dictionary_key_not_found(element->repr()));
    return error_status();
  }
  return ok_status();
}

starlark_obj* starlark_set::symmetric_difference(starlark_obj* other, context& ctx, error_fn& error_callback) const {
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  for (auto* value : values) {
    result->values.insert(value);
  }
  auto* it = other->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return nullptr;
  }
  while (it->has_next()) {
    auto* element = it->next();
    if (values.contains(element)) {
      result->values.erase(element);
    } else {
      if (!result->add(element, error_callback).ok()) {
        it->end_iterator();
        return nullptr;
      }
    }
  }
  it->end_iterator();
  return result;
}

status starlark_set::symmetric_difference_update(starlark_obj* other, context& ctx, error_fn& error_callback) {
  if (!can_modify("update", error_callback)) {
    return error_status();
  }
  if (other == this) {
    values.clear();
    return ok_status();
  }
  auto* it = other->get_iterator(true, ctx, error_callback);
  if (it == nullptr) {
    return error_status();
  }
  set_t original_values(values);
  while (it->has_next()) {
    auto* element = it->next();
    if (original_values.contains(element)) {
      values.erase(element);
    } else {
      if (!add(element, error_callback).ok()) {
        it->end_iterator();
        return error_status();
      }
    }
  }
  it->end_iterator();
  return ok_status();
}

starlark_obj* starlark_set::union_(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback) {
  auto* result = Arena::Create<starlark_set>(&ctx.arena());
  for (auto* value : values) {
    result->values.insert(value);
  }
  for (auto* other : others) {
    auto* it = other->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return nullptr;
    }
    while (it->has_next()) {
      if (!result->add(it->next(), error_callback).ok()) {
        it->end_iterator();
        return nullptr;
      }
    }
    it->end_iterator();
  }
  return result;
}

status starlark_set::update(const std::vector<starlark_obj*>& others, context& ctx, error_fn& error_callback) {
  if (!can_modify("append", error_callback)) {
    return error_status();
  }
  for (auto* other : others) {
    if (other == this) {
      continue;
    }
    auto* it = other->get_iterator(true, ctx, error_callback);
    if (it == nullptr) {
      return error_status();
    }
    while (it->has_next()) {
      if (!add(it->next(), error_callback).ok()) {
        it->end_iterator();
        return error_status();
      }
    }
    it->end_iterator();
  }
  return ok_status();
}

bool starlark_set::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_set* n_other = static_cast<const starlark_set*>(other);
  if (values.size() != n_other->values.size()) {
    return false;
  }
  for (const auto& element : values) {
    // This will not unboundly recurse as the element is hashable.
    if (!n_other->contains(element)) {
      return false;
    }
  }
  return true;
}

void starlark_set::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (extended && type() == other->type() && equals(*other)) {
    return;
  }
  starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_set::inner_hash() const {
  // My current understanding is that this is the right behavior.
  return -1;
}

void starlark_set::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto& element : values) {
    to_freeze.push_back(element);
  }
}

starlark_set::starlark_set_iterator::starlark_set_iterator(starlark_set* set) : set(set), it(set->values.begin()) {
  set->iterators_count++;
}

bool starlark_set::starlark_set_iterator::has_next() const {
  return it != set->values.end();
}

starlark_obj* starlark_set::starlark_set_iterator::next() {
  return *it++;
}

void starlark_set::starlark_set_iterator::end_iterator() {
  set->iterators_count--;
}

bool starlark_set::can_modify(std::string_view op, error_fn& error_callback) const {
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

starlark_obj* starlark_set_fn_add(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.add").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->add(pos_args.front(), error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_set_fn_clear(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "set.clear").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->clear(error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_set_fn_difference(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "set.difference").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  return static_cast<starlark_set*>(this_obj)->difference(pos_args, ctx, error_callback);
}

starlark_obj* starlark_set_fn_difference_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "set.difference_update").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->difference_update(pos_args, ctx, error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_set_fn_discard(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.discard").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->discard(pos_args.front(), error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_set_fn_intersection(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "set.intersection").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  return static_cast<starlark_set*>(this_obj)->intersection(pos_args, ctx, error_callback);
}

starlark_obj* starlark_set_fn_intersection_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "set.intersection_update").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->intersection_update(pos_args, ctx, error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_set_fn_isdisjoint(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.isdisjoint").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  auto result = static_cast<starlark_set*>(this_obj)->isdisjoint(pos_args.front(), ctx, error_callback);
  if (!result.ok()) {
    return nullptr;
  }
  return *result ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_set_fn_issubset(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.issubset").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  auto result = static_cast<starlark_set*>(this_obj)->issubset(pos_args.front(), ctx, error_callback);
  if (!result.ok()) {
    return nullptr;
  }
  return *result ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_set_fn_issuperset(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.issuperset").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  auto result = static_cast<starlark_set*>(this_obj)->issuperset(pos_args.front(), ctx, error_callback);
  if (!result.ok()) {
    return nullptr;
  }
  return *result ? ctx.true_value() : ctx.false_value();
}

starlark_obj* starlark_set_fn_pop(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "set.pop").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  return static_cast<starlark_set*>(this_obj)->pop(error_callback);
}

starlark_obj* starlark_set_fn_remove(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.remove").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->remove(pos_args.front(), error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_set_fn_symmetric_difference(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.symmetric_difference").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  return static_cast<starlark_set*>(this_obj)->symmetric_difference(pos_args.front(), ctx, error_callback);
}

starlark_obj* starlark_set_fn_symmetric_difference_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!one_pos_arg(pos_args, named_args, error_callback, "set.symmetric_difference_update").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->symmetric_difference_update(pos_args.front(), ctx, error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_set_fn_union(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "set.union").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  return static_cast<starlark_set*>(this_obj)->union_(pos_args, ctx, error_callback);
}

starlark_obj* starlark_set_fn_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_named_args(named_args, error_callback, "set.update").ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::set_t);
  if (!static_cast<starlark_set*>(this_obj)->update(pos_args, ctx, error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

}  // namespace runtime
}  // namespace starlark


