// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_dictionary.hpp"

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "errors/runtime_error_messages.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::error_messages::error_v2_dictionary_key_not_found;
using ::starlark::error_messages::error_v2_dictionary_update_sequence;
using ::starlark::error_messages::error_v2_empty_dictionary;
using ::starlark::error_messages::error_v2_mutate_frozen_value;
using ::starlark::error_messages::error_v2_op_in_loop;
using ::starlark::error_messages::error_v2_unhashable_key;
using ::starlark::error_messages::error_v2_unpack_too_few;
using ::starlark::error_messages::error_v2_unpack_too_many;
using ::starlark::result::error_status;
using ::starlark::result::ok_status;
using ::starlark::result::status;

namespace starlark {
namespace runtime {

starlark_obj* starlark_dictionary_fn_clear(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_get(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_items(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_keys(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_pop(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_popitem(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_setdefault(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);
starlark_obj* starlark_dictionary_fn_values(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback);

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_dictionary::method_refs() {
  static const std::map<std::string, starlark_obj::fn*, std::less<>>* result =
    new std::map<std::string, starlark_obj::fn*, std::less<>>{
      {"clear", starlark_dictionary_fn_clear},
      {"get", starlark_dictionary_fn_get},
      {"items", starlark_dictionary_fn_items},
      {"keys", starlark_dictionary_fn_keys},
      {"pop", starlark_dictionary_fn_pop},
      {"popitem", starlark_dictionary_fn_popitem},
      {"setdefault", starlark_dictionary_fn_setdefault},
      {"update", starlark_dictionary_fn_update},
      {"values", starlark_dictionary_fn_values},
    };

  return *result;
}

const std::vector<std::string>& starlark_dictionary::attributes() {
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

starlark_dictionary::starlark_dictionary() : iterators_count(0) {}

std::string_view starlark_dictionary::type() const {
  return starlark_types::dict_t;
}

const std::vector<std::string>& starlark_dictionary::dir() const {
  return attributes();
}

const std::map<std::string, starlark_obj::fn*, std::less<>>& starlark_dictionary::methods_meta() const {
  return method_refs();
}

void starlark_dictionary::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, context& ctx, error_fn& error_callback) {
  if (number_of_elements != values_.size()) {
    if (values_.size() < number_of_elements) {
      error_callback.add_error(error_v2_unpack_too_few(values_.size(), number_of_elements));
    } else {
      error_callback.add_error(error_v2_unpack_too_many(values_.size(), number_of_elements));
    }
    return;
  }
  for (auto it = values_.rbegin(); it != values_.rend(); ++it) {
    consumer.push_back(it->first);
  }
}

int64_t starlark_dictionary::len(bool produce_error, error_fn& error_callback) const {
  return values_.size();
}

bool starlark_dictionary::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::kPrintTop: {
      if (values_.size() == 0) {
        print.append("{}");
        return false;
      }
      print.append("{");
      printer_action new_action = printer_action::kPrintFinal;
      for (auto it = values_.rbegin(); it != values_.rend(); ++it) {
        print.add_task(printer::pending_task{
          .obj = this,
          .action = new_action,
        });
        print.add_task(printer::pending_task{
          .obj = it->second,
          .action = printer_action::kPrintTop,
        });
        print.add_task(printer::pending_task{
          .obj = this,
          .action = printer_action::kPrintInElementSeparator,
        });
        print.add_task(printer::pending_task{
          .obj = it->first,
          .action = printer_action::kPrintTop,
        });
        new_action = printer_action::kPrintElementSeparator;
      }
      return true;
    }
    case printer_action::kPrintElementSeparator:
      print.append(", ");
      return true;
    case printer_action::kPrintInElementSeparator:
      print.append(": ");
      return true;
    case printer_action::kPrintFinal:
      print.append("}");
      return false;
    case printer_action::kPrintRecursion:
    default:
      print.append("{...}");
      return false;
  }
}

bool starlark_dictionary::truthy() const {
  return !values_.empty();
}

bool starlark_dictionary::binary_in(const starlark_obj& other, error_fn& error_callback) const {
  if (other.hash() == -1) {
    error_callback.add_error(error_v2_unhashable_key(type(), other.type()));
    return false;
  }
  // The const_cast is needed as there is no conversion from `const starlark_obj *const` to `starlark_obj *const`.
  return values_.contains(&const_cast<starlark_obj&>(other));
}

starlark_obj* starlark_dictionary::binary_pipe(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  if (other.type() != type()) {
    return starlark_obj::binary_pipe(other, ctx, error_callback);
  }
  auto* result = Arena::Create<starlark_dictionary>(&ctx.arena());
  for (const auto& [key, value] : values_) {
    result->insert(key, value, error_callback);
  }
  const starlark_dictionary* d_other = static_cast<const starlark_dictionary*>(&other);
  for (auto& [key, value] : d_other->values_) {
    result->insert(key, value, error_callback);
  }
  return result;
}

starlark_obj* starlark_dictionary::pipe_equals_assign(const starlark_obj& other, context& ctx, error_fn& error_callback) {
  if (!can_modify("append", error_callback)) {
    return nullptr;
  }
  if (other.type() != type()) {
    return starlark_obj::pipe_equals_assign(other, ctx, error_callback);
  }
  const starlark_dictionary* d_other = static_cast<const starlark_dictionary*>(&other);
  for (auto& [key, value] : d_other->values_) {
    insert(key, value, error_callback);
  }
  return this;
}

starlark_iterator* starlark_dictionary::get_iterator(bool produce_error, context& ctx, error_fn& error_callback) {
  return Arena::Create<starlark_dictionary_iterator>(&ctx.arena(), this);
}

starlark_obj* starlark_dictionary::index(const starlark_obj& other, context& ctx, error_fn& error_callback) const {
  if (other.hash() == -1) {
    error_callback.add_error(error_v2_unhashable_key(type(), other.type()));
    return nullptr;
  }
  auto result = values_.find(&const_cast<starlark_obj&>(other));
  if (result == values_.end()) {
    error_callback.add_error(error_v2_dictionary_key_not_found(other.repr()));
    return nullptr;
  }
  return result->second;
}

void starlark_dictionary::index_assign(const starlark_obj& idx, starlark_obj& element, error_fn& error_callback) {
  if (!can_modify("assign", error_callback)) {
    return;
  }
  if (idx.hash() == -1) {
    error_callback.add_error(error_v2_unhashable_key(type(), idx.type()));
    return;
  }
  values_.insert(&const_cast<starlark_obj&>(idx), &element);
}

status starlark_dictionary::clear(error_fn& error_callback) {
  if (!can_modify("clear", error_callback)) {
    return error_status();
  }
  values_.clear();
  return ok_status();
}

starlark_obj* starlark_dictionary::get(starlark_obj* key, starlark_obj* default_value, error_fn& error_callback) const {
  if (key->hash() == -1) {
    error_callback.add_error(error_v2_unhashable_key(type(), key->type()));
    return nullptr;
  }
  auto it = values_.find(key);
  if (it == values_.end()) {
    return default_value;
  }
  return it->second;
}

starlark_obj* starlark_dictionary::items(context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), values_.size());
  for (auto entry : values_) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), 2);
    tuple->add(entry.first);
    tuple->add(entry.second);
    result->unsafe_append(tuple);
  }
  return result;
}

starlark_obj* starlark_dictionary::keys(context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), values_.size());
  for (auto entry : values_) {
    result->unsafe_append(entry.first);
  }
  return result;
}

starlark_obj* starlark_dictionary::pop(starlark_obj* key, starlark_obj* default_value, error_fn& error_callback) {
  if (!can_modify("pop", error_callback)) {
    return nullptr;
  }
  if (key->hash() == -1) {
    error_callback.add_error(error_v2_unhashable_key(type(), key->type()));
    return nullptr;
  }
  auto it = values_.find(key);
  if (it == values_.end()) {
    if (default_value == nullptr) {
      error_callback.add_error(error_v2_dictionary_key_not_found(key->repr()));
    }
    return default_value;
  }
  auto* result = it->second;
  values_.erase(key);
  return result;
}

starlark_obj* starlark_dictionary::popitem(context& ctx, error_fn& error_callback) {
  if (!can_modify("popitem", error_callback)) {
    return nullptr;
  }
  if (values_.empty()) {
    error_callback.add_error(error_v2_empty_dictionary("popitem"));
    return nullptr;
  }
  starlark_tuple* result = Arena::Create<starlark_tuple>(&ctx.arena(), 2);
  auto it = values_.begin();
  result->add(it->first);
  result->add(it->second);
  values_.erase(it->first);
  return result;
}

starlark_obj* starlark_dictionary::setdefault(starlark_obj* key, starlark_obj* default_value, error_fn& error_callback) {
  if (!can_modify("setdefault", error_callback)) {
    return nullptr;
  }
  if (key->hash() == -1) {
    error_callback.add_error(error_v2_unhashable_key(type(), key->type()));
    return nullptr;
  }
  auto it = values_.find(key);
  if (it != values_.end()) {
    return it->second;
  }
  values_.insert(key, default_value);
  return default_value;
}

status starlark_dictionary::update(starlark_obj* iterable, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!can_modify("update", error_callback)) {
    return error_status();
  }
  if (iterable != nullptr) {
    // This is a special case. This should be extended to understand any mapping, but at the moment only `dictionary` implements it.
    if (iterable->type() == starlark_types::dict_t) {
      starlark_dictionary* d_iterable = static_cast<starlark_dictionary*>(iterable);
      for (auto& kv : d_iterable->values_) {
        values_.insert(kv.first, kv.second);
      }
    } else {
      int pos = 0;
      auto* it = iterable->get_iterator(true, ctx, error_callback);
      if (it == nullptr) {
        return error_status();
      }
      while (it->has_next()) {
        auto* kv = it->next();
        assert(kv != nullptr);
        auto* it2 = kv->get_iterator(true, ctx, error_callback);
        if (it2 == nullptr) {
          return error_status();
        }
        if (!it2->has_next()) {
          error_callback.add_error(error_v2_dictionary_update_sequence(pos, 0, 2));
          return error_status();
        }
        auto* key = it2->next();
        assert(key != nullptr);
        if (!it2->has_next()) {
          error_callback.add_error(error_v2_dictionary_update_sequence(pos, 1, 2));
          return error_status();
        }
        auto* value = it2->next();
        assert(value != nullptr);
        if (it2->has_next()) {
          error_callback.add_error(error_v2_dictionary_update_sequence(pos, kv->len(false, error_callback), 2));
          return error_status();
        }
        if (insert(key, value, error_callback).second) {
          return error_status();
        }
        it2->end_iterator();
        pos++;
      }
      it->end_iterator();
    }
  }
  for (auto& [key, value] : named_args) {
    values_.insert(Arena::Create<starlark_string>(&ctx.arena(), key), value);
  }
  return ok_status();
}

starlark_obj* starlark_dictionary::values(context& ctx) const {
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), values_.size());
  for (auto entry : values_) {
    result->unsafe_append(entry.second);
  }
  return result;
}

bool starlark_dictionary::inner_equals(equals_comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_dictionary* n_other = reinterpret_cast<const starlark_dictionary*>(other);
  if (values_.size() != n_other->values_.size()) {
    return false;
  }
  for (const auto& element : values_) {
    auto other_element = n_other->values_.find(element.first);
    if (other_element == n_other->values_.end()) {
      return false;
    }
    comp.add_task(equals_comparator::pending_task{
      .lhs = element.second,
      .rhs = other_element->second,
    });
  }
  return true;
}

void starlark_dictionary::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (extended && type() == other->type() && equals(*other)) {
    return;
  }
  starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
}

void starlark_dictionary::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto& [k, v] : values_) {
    to_freeze.push_back(k);
    to_freeze.push_back(v);
  }
}

std::variant<int64_t, starlark_obj::pending_hash> starlark_dictionary::inner_hash() const {
  // My current understanding is that this is the right behavior.
  return -1;
}

std::pair<bool, bool> starlark_dictionary::insert(starlark_obj* key, starlark_obj* value, error_fn& error_callback) {
  // TODO(lmirelmann): check whether this can be replaced with `insert_unsafe`.
  if (!can_modify("insert", error_callback)) {
    return std::make_pair(false, true);
  }
  if (key->hash() == -1) {
    error_callback.add_error(error_v2_unhashable_key(type(), key->type()));
    return std::make_pair(false, true);
  }
  auto [it, result] = values_.insert(key, value);
  return std::make_pair(result, false);
}

starlark_dictionary::starlark_dictionary_iterator::starlark_dictionary_iterator(starlark_dictionary* dictionary) : dictionary(dictionary), it(dictionary->values_.begin()) {
  dictionary->iterators_count++;
}

bool starlark_dictionary::starlark_dictionary_iterator::has_next() const {
  return it != dictionary->values_.end();
}

starlark_obj* starlark_dictionary::starlark_dictionary_iterator::next() {
  return (it++)->first;
}

void starlark_dictionary::starlark_dictionary_iterator::end_iterator() {
  dictionary->iterators_count--;
}

bool starlark_dictionary::can_modify(std::string_view op, error_fn& error_callback) const {
  if (iterators_count) {
    error_callback.add_error(error_v2_op_in_loop(type(), op));
    return false;
  }
  if (freezed) {
    error_callback.add_error(error_v2_mutate_frozen_value(type()));
    return false;
  }
  return true;
}

starlark_obj* starlark_dictionary_fn_clear(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "dict.clear").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  if (!static_cast<starlark_dictionary*>(this_obj)->clear(error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_dictionary_fn_get(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // Bazel allows the second parameter to be named with name `default`. This is not allowed in Python.
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  if (!no_named_args(named_args, error_callback, "dict.get").ok() ||
      !min_args(pos_args, error_callback, "get", 1).ok() ||
      !max_args(pos_args, error_callback, "get", 2).ok()) {
    return nullptr;
  }
  starlark_obj* default_value = pos_args.size() == 2 ? pos_args[1] : ctx.none_value();
  return static_cast<starlark_dictionary*>(this_obj)->get(pos_args.front(), default_value, error_callback);
}

starlark_obj* starlark_dictionary_fn_items(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "dict.items").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  return static_cast<starlark_dictionary*>(this_obj)->items(ctx);
}

starlark_obj* starlark_dictionary_fn_keys(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "dict.keys").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  return static_cast<starlark_dictionary*>(this_obj)->keys(ctx);
}

starlark_obj* starlark_dictionary_fn_pop(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // Bazel allows the second paramter to be a named argument with name `unbound`. This is not allowed in Python. In the spec and Python the name of the argument is called `default`.
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  if (!no_named_args(named_args, error_callback, "dict.pop").ok() ||
      !min_args(pos_args, error_callback, "pop", 1).ok() ||
      !max_args(pos_args, error_callback, "pop", 2).ok()) {
    return nullptr;
  }
  starlark_dictionary* dict = static_cast<starlark_dictionary*>(this_obj);
  return dict->pop(pos_args.front(), pos_args.size() > 1 ? pos_args[1] : nullptr, error_callback);
}

starlark_obj* starlark_dictionary_fn_popitem(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "dict.popitem").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  return static_cast<starlark_dictionary*>(this_obj)->popitem(ctx, error_callback);
}

starlark_obj* starlark_dictionary_fn_setdefault(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  // Bazel allows the second paramter to be a named argument with name `unbound`. This is not allowed in Python. In the spec and Python the name of the argument is called `default`.
  if (!no_named_args(named_args, error_callback, "dict.setdefault").ok() ||
      !min_args(pos_args, error_callback, "setdefault", 1).ok() ||
      !max_args(pos_args, error_callback, "setdefault", 2).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  auto* default_value = pos_args.size() > 1 ? pos_args[1] : ctx.none_value();
  return static_cast<starlark_dictionary*>(this_obj)->setdefault(pos_args.front(), default_value, error_callback);
}

starlark_obj* starlark_dictionary_fn_update(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!max_args(pos_args, error_callback, "update", 1).ok()) {
    return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  starlark_obj* pos_arg = pos_args.empty() ? nullptr : pos_args.front();
  if (!static_cast<starlark_dictionary*>(this_obj)->update(pos_arg, named_args, ctx, error_callback).ok()) {
    return nullptr;
  }
  return ctx.none_value();
}

starlark_obj* starlark_dictionary_fn_values(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  if (!no_arg(pos_args, named_args, error_callback, "dict.values").ok()) {
     return nullptr;
  }
  assert(this_obj != nullptr);
  assert(this_obj->type() == starlark_types::dict_t);
  return static_cast<starlark_dictionary*>(this_obj)->values(ctx);
}

}  // namespace runtime
}  // namespace starlark


