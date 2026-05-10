// Copyright 2026 Lucas Mirelmann

#include "interpreter/built_in_functions.hpp"

#include <cassert>

#include <vector>
#include <utility>

#include "runtime/error_messages.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
using ::starlark::runtime::error_empty_iterator;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::error_named_argument_must_be_type;
using ::starlark::runtime::error_unknown_argument;
using ::starlark::runtime::starlark_built_in_functions;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_types;

namespace starlark {
namespace interpreter {

starlark_obj* starlark_fn_inner_max(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  assert(pos_args.size() == 2);
  assert(named_args.empty());
  auto* it1 = pos_args.front()->get_iterator(true, ctx, error_callback);
  assert(it1 != nullptr);
  if (!it1->has_next()) {
    it1->end_iterator();
    error_callback.add_error(error_empty_iterator(starlark_built_in_functions::max_f));
    return nullptr;
  }
  auto* it2 = pos_args[1]->get_iterator(true, ctx, error_callback);
  assert(it2 != nullptr);
  assert(it2->has_next());
  starlark_obj* candidate = it1->next();
  const starlark_obj* candidate_key = it2->next();
  while (it1->has_next()) {
    auto* element = it1->next();
    assert(it2->has_next());
    const starlark_obj* element_key = it2->next();
    auto cmp = candidate_key->cmp(*element_key, "<", error_callback);
    if (!cmp.ok()) {
      return nullptr;
    }
    if (*cmp < 0) {
      candidate = element;
      candidate_key = element_key;
    }
  }
  assert(!it2->has_next());
  it1->end_iterator();
  it2->end_iterator();
  return candidate;
}

starlark_obj* starlark_fn_inner_min(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  assert(pos_args.size() == 2);
  assert(named_args.empty());
  auto* it1 = pos_args.front()->get_iterator(true, ctx, error_callback);
  assert(it1 != nullptr);
  if (!it1->has_next()) {
    it1->end_iterator();
    error_callback.add_error(error_empty_iterator(starlark_built_in_functions::max_f));
    return nullptr;
  }
  auto* it2 = pos_args[1]->get_iterator(true, ctx, error_callback);
  assert(it2 != nullptr);
  assert(it2->has_next());
  starlark_obj* candidate = it1->next();
  const starlark_obj* candidate_key = it2->next();
  while (it1->has_next()) {
    auto* element = it1->next();
    assert(it2->has_next());
    const starlark_obj* element_key = it2->next();
    auto cmp = candidate_key->cmp(*element_key, "<", error_callback);
    if (!cmp.ok()) {
      return nullptr;
    }
    if (*cmp > 0) {
      candidate = element;
      candidate_key = element_key;
    }
  }
  assert(!it2->has_next());
  it1->end_iterator();
  it2->end_iterator();
  return candidate;
}

starlark_obj* starlark_fn_inner_sorted(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  assert(pos_args.size() == 3);
  bool reverse = pos_args[2]->truthy();

  // Retrieve the elements.
  auto* it1 = pos_args.front()->get_iterator(true, ctx, error_callback);
  assert(it1 != nullptr);
  auto* it2 = pos_args[1]->get_iterator(true, ctx, error_callback);
  assert(it2 != nullptr);
  std::vector<std::pair<starlark_obj*, starlark_obj*>> elems;
  while (it1->has_next()) {
    assert(it2->has_next());
    starlark_obj* element = it1->next();
    starlark_obj* element_key = it2->next();
    elems.emplace_back(element_key, element);
  }
  assert(!it2->has_next());
  it1->end_iterator();
  it2->end_iterator();

  // Sort and manybe revert.
  bool found_error = false;
  std::stable_sort(elems.begin(), elems.end(), [&error_callback, &found_error](const std::pair<starlark_obj*, starlark_obj*>& lhs, const std::pair<starlark_obj*, starlark_obj*>& rhs) -> bool {
    if (found_error) {
      return false;
    }
    auto cmp = lhs.first->cmp(*rhs.first, "<", error_callback);
    if (!cmp.ok()) {
      found_error = true;
      return false;
    }
    return *cmp < 0;
  });
  if (found_error) {
    return nullptr;
  }
  if (reverse) {
    std::reverse(elems.begin(), elems.end());
  }

  // Create the output.
  auto* result = Arena::Create<starlark_list>(&ctx.arena(), elems.size());
  for (auto& elem : elems) {
    result->unsafe_append(elem.second);
  }
  return result;
}

}  // namespace interpreter
}  // namespace starlark

