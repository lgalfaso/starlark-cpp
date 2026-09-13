// Copyright 2026 Lucas Mirelmann

#include "interpreter/function.hpp"

#include <format>
#include <span>
#include <string>
#include <utility>
#include <vector>

#include "containers/linked_hash_map.hpp"
#include "errors/runtime_error_messages.hpp"
#include "interpreter/runner_state.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"
#include "string/levenshtein.hpp"
#include "vm/module_metadata.hpp"

using ::google::protobuf::Arena;
using ::starlark::error_messages::error_v2_arguments_exactly;
using ::starlark::error_messages::error_v2_missing_keyword_only_argument;
using ::starlark::error_messages::error_v2_missing_positional_argument;
using ::starlark::error_messages::error_v2_module_does_not_define_symbol;
using ::starlark::error_messages::error_v2_multiple_values_for_argument;
using ::starlark::error_messages::error_v2_recursive_call;
using ::starlark::error_messages::error_v2_unable_to_load_module;
using ::starlark::error_messages::error_v2_unexpected_keyword_argument;
using ::starlark::error_messages::error_v2_unexpected_keyword_argument_with_hint;
using ::starlark::runtime::context;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::order_comparator;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::vm::builtin_star_module;
using ::starlark::vm::frame;
using ::starlark::vm::function_metadata_for_block;

using kwargs_map_t = starlark::cnt::linked_hash_map<std::string_view, starlark_obj*, std::hash<std::string_view>, std::equal_to<std::string_view>>;

namespace starlark {
namespace interpreter {

std::strong_ordering cmp_fn(const interpreter_function* lhs, const interpreter_function* rhs, bool compare_stack) {
  auto c = lhs->entrypoint <=> rhs->entrypoint;
  if (c != 0) {
    return c;
  }
  c = lhs->current_program.bytecode <=> rhs->current_program.bytecode;
  if (c != 0) {
    return c;
  }
  if (compare_stack) {
    c = lhs->frame_stack.size() <=> rhs->frame_stack.size();
    if (c != 0) {
      return c;
    }
    for (std::size_t i = 0; i < lhs->frame_stack.size(); ++i) {
      c = lhs->frame_stack[i] <=> rhs->frame_stack[i];
      if (c != 0) {
        return c;
      }
    }
  }
  return c;
}

bool less_fn::operator()(const interpreter_function* lhs, const interpreter_function* rhs) const {
  return cmp_fn(lhs, rhs, false) < 0;
}

interpreter_function::interpreter_function(
    int entrypoint,
    std::vector<starlark::runtime::starlark_obj*>&& default_arguments,
    const runner_state::program_info& current_program_,
    bool inner_fn,
    std::string_view fn_name,
    const google::protobuf::RepeatedPtrField<std::string>* frame_names,
    const std::vector<frame*>& frame_stack) :
      starlark::runtime::starlark_function(
          fn_name,
          current_program_.module_name,
          starlark::runtime::object_kind::kFunction),
      entrypoint(entrypoint),
      default_arguments(default_arguments),
      frame_names(frame_names),
      frame_stack(frame_stack),
      current_program(current_program_),
      inner_(inner_fn) {
  default_parameters = nullptr;
  assert(fn_meta() != nullptr);
  for (std::size_t i = 0; i < fn_meta()->params.size(); ++i) {
    named_argument_index[fn_meta()->params[i].name] = i;
  }
  compute_simple_call_metadata();
}

const starlark::vm::function_signature_metadata* interpreter_function::fn_meta() const {
  assert(current_program.metadata != nullptr);
  return function_metadata_for_block(*current_program.metadata, entrypoint);
}

void interpreter_function::compute_simple_call_metadata() {
  simple_positional_arity_ = -1;
  positional_param_frame_pos_.clear();
  keyword_only_param_start_ = 0;
  keyword_only_param_count_ = 0;
  keyword_only_default_offset_ = 0;

  assert(fn_meta() != nullptr);
  if (fn_meta()->has_star_argument || fn_meta()->has_star_star_argument) {
    return;
  }
  const int number_of_standard_params = fn_meta()->param_count;
  const int keyword_only_count = fn_meta()->keyword_only_parameter_count;
  const int number_positional_params = fn_meta()->positional_param_count;
  // Positional-only fast path (call_pos / CallPos*) cannot supply keyword-only
  // arguments. Skip optimization when any keyword-only param lacks a default.
  for (int i = number_positional_params; i < number_of_standard_params; ++i) {
    if (!fn_meta()->params[i].default_initialization) {
      return;
    }
  }
  simple_positional_arity_ = number_positional_params;
  positional_param_frame_pos_.resize(number_positional_params);
  for (int i = 0; i < number_positional_params; ++i) {
    positional_param_frame_pos_[i] = fn_meta()->params[i].pos_in_frame;
    if (fn_meta()->params[i].default_initialization) {
      keyword_only_default_offset_++;
    }
  }
  keyword_only_param_start_ = number_positional_params;
  keyword_only_param_count_ = keyword_only_count;
}

bool interpreter_function::begin_call(runner_state* state, context& ctx, error_fn& error_callback) {
  if (!inner_ && !ctx.options().allow_recursion) {
    if (state->fns_in_stack.contains(this)) {
      error_callback.add_error(error_v2_recursive_call(fn_name));
      return false;
    }
  }
  state->fns_in_stack[this]++;
  state->call_fns.push_back(this);
  return true;
}

void interpreter_function::commit_frame(frame* new_frame, runner_state* state, context& ctx) {
  state->frame_stacks.push_back(frame_stack);
  state->frame_stacks.back().push_back(new_frame);
  state->call_stack.push_back(call_stack_entry{.block_ptr = state->block_ptr, .instruction_ptr = state->instruction_ptr, .inner = state->inner});
  state->block_ptr = entrypoint;
  state->instruction_ptr = 0;
  state->inner = inner_;
  state->current_program_stack.push_back(state->current_program);
  state->current_program = current_program;
  refresh_current_op_codes(*state);
}

void interpreter_function::apply_keyword_only_defaults(frame* new_frame) {
  if (keyword_only_param_count_ == 0) {
    return;
  }
  assert(fn_meta() != nullptr);
  int default_argument_pos = keyword_only_default_offset_;
  for (int i = 0; i < keyword_only_param_count_; ++i) {
    const int param_idx = keyword_only_param_start_ + i;
    new_frame->elements[fn_meta()->params[param_idx].pos_in_frame] = default_arguments[default_argument_pos];
    default_argument_pos++;
  }
}

starlark_obj* interpreter_function::call_pos_fixed_0(context& ctx, error_fn& error_callback) {
  assert(simple_positional_arity_ == 0);
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  if (!begin_call(state, ctx, error_callback)) {
    return nullptr;
  }
  auto* new_frame = Arena::Create<frame>(&ctx.arena(), frame_names);
  apply_keyword_only_defaults(new_frame);
  commit_frame(new_frame, state, ctx);
  return ctx.none_value();
}

starlark_obj* interpreter_function::call_pos_fixed_1(starlark_obj* arg0, context& ctx, error_fn& error_callback) {
  assert(simple_positional_arity_ == 1);
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  if (!begin_call(state, ctx, error_callback)) {
    return nullptr;
  }
  auto* new_frame = Arena::Create<frame>(&ctx.arena(), frame_names);
  new_frame->elements[positional_param_frame_pos_[0]] = arg0;
  apply_keyword_only_defaults(new_frame);
  commit_frame(new_frame, state, ctx);
  return ctx.none_value();
}

starlark_obj* interpreter_function::call_pos_fixed_2(starlark_obj* arg0, starlark_obj* arg1, context& ctx, error_fn& error_callback) {
  assert(simple_positional_arity_ == 2);
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  if (!begin_call(state, ctx, error_callback)) {
    return nullptr;
  }
  auto* new_frame = Arena::Create<frame>(&ctx.arena(), frame_names);
  new_frame->elements[positional_param_frame_pos_[0]] = arg0;
  new_frame->elements[positional_param_frame_pos_[1]] = arg1;
  apply_keyword_only_defaults(new_frame);
  commit_frame(new_frame, state, ctx);
  return ctx.none_value();
}

starlark_obj* interpreter_function::call_pos_fixed_3(
    starlark_obj* arg0,
    starlark_obj* arg1,
    starlark_obj* arg2,
    context& ctx,
    error_fn& error_callback) {
  assert(simple_positional_arity_ == 3);
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  if (!begin_call(state, ctx, error_callback)) {
    return nullptr;
  }
  auto* new_frame = Arena::Create<frame>(&ctx.arena(), frame_names);
  new_frame->elements[positional_param_frame_pos_[0]] = arg0;
  new_frame->elements[positional_param_frame_pos_[1]] = arg1;
  new_frame->elements[positional_param_frame_pos_[2]] = arg2;
  apply_keyword_only_defaults(new_frame);
  commit_frame(new_frame, state, ctx);
  return ctx.none_value();
}

starlark_obj* interpreter_function::call_pos(
    std::span<starlark_obj*> pos_args,
    context& ctx,
    error_fn& error_callback) {
  if (simple_positional_arity_ >= 0 && pos_args.size() == static_cast<std::size_t>(simple_positional_arity_)) {
    switch (simple_positional_arity_) {
      case 0:
        return call_pos_fixed_0(ctx, error_callback);
      case 1:
        return call_pos_fixed_1(pos_args[0], ctx, error_callback);
      case 2:
        return call_pos_fixed_2(pos_args[0], pos_args[1], ctx, error_callback);
      case 3:
        return call_pos_fixed_3(pos_args[0], pos_args[1], pos_args[2], ctx, error_callback);
      default:
        break;
    }
  }
  return call_pos_general(pos_args, ctx, error_callback);
}

starlark_obj* interpreter_function::call_pos_general(
    std::span<starlark_obj*> pos_args,
    context& ctx,
    error_fn& error_callback) {
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  if (!inner_ && !ctx.options().allow_recursion) {
    if (state->fns_in_stack.contains(this)) {
      error_callback.add_error(error_v2_recursive_call(fn_name));
      return nullptr;
    }
  }
  state->fns_in_stack[this]++;
  state->call_fns.push_back(this);
  auto* new_frame = Arena::Create<frame>(&ctx.arena(), frame_names);
  starlark_obj::pos_args_t star_args;
  int next_positional_param = 0;
  assert(fn_meta() != nullptr);
  const int number_positional_params = fn_meta()->positional_param_count;

  std::vector<bool> filled_elements(frame_names->size());
  for (auto* param : pos_args) {
    if (next_positional_param < number_positional_params) {
      auto& signature_param = fn_meta()->params[next_positional_param];
      new_frame->elements[signature_param.pos_in_frame] = param;
      filled_elements[next_positional_param] = true;
      next_positional_param++;
    } else if (fn_meta()->has_star_argument) {
      star_args.push_back(param);
    } else {
      error_callback.add_error(error_v2_arguments_exactly(
          fn_name,
          pos_args.size(),
          number_positional_params));
      return nullptr;
    }
  }
  int default_argument_pos = 0;
  for (int i = 0; i < number_positional_params; ++i) {
    if (!filled_elements[i]) {
      if (fn_meta()->params[i].default_initialization) {
        new_frame->elements[fn_meta()->params[i].pos_in_frame] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_v2_missing_positional_argument(
            fn_name,
            fn_meta()->params[i].name));
        return nullptr;
      }
    }
    if (fn_meta()->params[i].default_initialization) {
      default_argument_pos++;
    }
  }
  auto keyword_only_parameter_start = number_positional_params;
  if (fn_meta()->has_star_argument) {
    keyword_only_parameter_start++;
  }
  for (int i = keyword_only_parameter_start; i < keyword_only_parameter_start + fn_meta()->keyword_only_parameter_count; ++i) {
    if (!filled_elements[i]) {
      if (fn_meta()->params[i].default_initialization) {
        new_frame->elements[fn_meta()->params[i].pos_in_frame] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_v2_missing_keyword_only_argument(
            fn_name,
            fn_meta()->params[i].name));
        return nullptr;
      }
    }
    if (fn_meta()->params[i].default_initialization) {
      default_argument_pos++;
    }
  }
  if (fn_meta()->has_star_argument) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), star_args.size());
    for (auto* element : star_args) {
      tuple->add(element);
    }
    new_frame->elements[fn_meta()->params[number_positional_params].pos_in_frame] = tuple;
  }
  if (fn_meta()->has_star_star_argument) {
    auto* dict = Arena::Create<starlark_dictionary>(&ctx.arena());
    new_frame->elements[fn_meta()->params[fn_meta()->param_count - 1].pos_in_frame] = dict;
  }

  state->frame_stacks.push_back(frame_stack);
  state->frame_stacks.back().push_back(new_frame);
  state->call_stack.push_back(call_stack_entry{.block_ptr = state->block_ptr, .instruction_ptr = state->instruction_ptr, .inner = state->inner});
  state->block_ptr = entrypoint;
  state->instruction_ptr = 0;
  state->inner = inner_;
  state->current_program_stack.push_back(state->current_program);
  state->current_program = current_program;
  refresh_current_op_codes(*state);
  return ctx.none_value();
}

starlark_obj* interpreter_function::call(
      const starlark_obj::pos_args_t& pos_args,
      const starlark_obj::named_args_t& named_args,
      context& ctx,
      error_fn& error_callback) {
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  // Inner functions can be called recursivelly.
  // Context: https://github.com/bazelbuild/bazel/issues/29920
  if (!inner_ && !ctx.options().allow_recursion) {
    if (state->fns_in_stack.contains(this)) {
      error_callback.add_error(error_v2_recursive_call(fn_name));
      return nullptr;
    }
  }
  state->fns_in_stack[this]++;
  state->call_fns.push_back(this);
  auto* new_frame = Arena::Create<frame>(&ctx.arena(), frame_names);
  starlark_obj::pos_args_t args;
  kwargs_map_t kwargs;
  int next_positional_param = 0;
  assert(fn_meta() != nullptr);
  const int number_positional_params = fn_meta()->positional_param_count;

  std::vector<bool> filled_elements(frame_names->size());
  // Process the positional arguments.
  for (auto* param : pos_args) {
    if (next_positional_param < number_positional_params) {
      auto& signature_param = fn_meta()->params[next_positional_param];
      new_frame->elements[signature_param.pos_in_frame] = param;
      filled_elements[next_positional_param] = true;
      next_positional_param++;
    } else if (fn_meta()->has_star_argument) {
      args.push_back(param);
    } else {
      error_callback.add_error(error_v2_arguments_exactly(
          fn_name,
          pos_args.size(),
          number_positional_params));
      return nullptr;
    }
  }
  // Process the named arguments.
  for (auto& kwparam : named_args) {
    auto it = named_argument_index.find(kwparam.first->as_string());
    if (it == named_argument_index.end()) {
      if (fn_meta()->has_star_star_argument) {
        kwargs.insert(kwparam.first->as_string(), kwparam.second);
      } else {
        std::vector<std::string> all_candidates;
        for (const auto& param : fn_meta()->params) {
          all_candidates.emplace_back(param.name);
        }
        auto candidate = starlark::string::levenshtein(kwparam.first->as_string(), all_candidates);
        if (candidate >= 0) {
          error_callback.add_error(error_v2_unexpected_keyword_argument_with_hint(
              fn_name,
              kwparam.first->as_string(),
              all_candidates[candidate]), all_candidates[candidate]);
        } else {
          error_callback.add_error(error_v2_unexpected_keyword_argument(
              fn_name,
              kwparam.first->as_string()));
        }
        return nullptr;
      }
      continue;
    }
    auto pos = it->second;
    if (filled_elements[pos]) {
      error_callback.add_error(error_v2_multiple_values_for_argument(
          fn_name,
          kwparam.first->as_string()));
      return nullptr;
    }
    new_frame->elements[fn_meta()->params[pos].pos_in_frame] = kwparam.second;
    filled_elements[pos] = true;
  }
  // Put the default arguments, and check that all the slots are filled on positional arguments.
  int default_argument_pos = 0;
  for (int i = 0; i < number_positional_params; ++i) {
    if (!filled_elements[i]) {
      if (fn_meta()->params[i].default_initialization) {
        new_frame->elements[fn_meta()->params[i].pos_in_frame] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_v2_missing_positional_argument(
            fn_name,
            fn_meta()->params[i].name));
        return nullptr;
      }
    }
    if (fn_meta()->params[i].default_initialization) {
      default_argument_pos++;
    }
  }
  // Put the default arguments, and check that all the slots are filled on keyword-only arguments.
  auto keyword_only_parameter_start = number_positional_params;
  if (fn_meta()->has_star_argument) {
    keyword_only_parameter_start++;
  }
  for (int i = keyword_only_parameter_start; i < keyword_only_parameter_start + fn_meta()->keyword_only_parameter_count; ++i) {
    if (!filled_elements[i]) {
      if (fn_meta()->params[i].default_initialization) {
        new_frame->elements[fn_meta()->params[i].pos_in_frame] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_v2_missing_keyword_only_argument(
            fn_name,
            fn_meta()->params[i].name));
        return nullptr;
      }
    }
    if (fn_meta()->params[i].default_initialization) {
      default_argument_pos++;
    }
  }
  // Fill *args.
  if (fn_meta()->has_star_argument) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), args.size());
    for (auto* element : args) {
      tuple->add(element);
    }
    new_frame->elements[fn_meta()->params[number_positional_params].pos_in_frame] = tuple;
  }
  // Fill **kwargs.
  if (fn_meta()->has_star_star_argument) {
    auto* dict = Arena::Create<starlark_dictionary>(&ctx.arena());
    for (auto& element : kwargs) {
      dict->insert(Arena::Create<starlark_string>(&ctx.arena(), element.first), element.second, error_callback);
    }
    new_frame->elements[fn_meta()->params[fn_meta()->param_count - 1].pos_in_frame] = dict;
  }

  state->frame_stacks.push_back(frame_stack);
  state->frame_stacks.back().push_back(new_frame);
  state->call_stack.push_back(call_stack_entry{.block_ptr = state->block_ptr, .instruction_ptr = state->instruction_ptr, .inner = state->inner});
  state->block_ptr = entrypoint;
  state->instruction_ptr = 0;
  state->inner = inner_;
  state->current_program_stack.push_back(state->current_program);
  state->current_program = current_program;
  refresh_current_op_codes(*state);
  return ctx.none_value();
}

bool interpreter_function::inner_equals(starlark::runtime::equals_comparator& comp, const starlark_obj* other) const {
  if (!same_starlark_type(other->kind(), kind())) {
    return false;
  }
  const interpreter_function* f_other = reinterpret_cast<const interpreter_function*>(other);
  return cmp_fn(this, f_other, true) == 0;
}

void interpreter_function::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (extended && same_starlark_type(kind(), other->kind()) && equals(*other)) {
    return;
  }
  starlark_obj::inner_cmp(comp, other, op, extended, error_callback);
}

void interpreter_function::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto* element : default_arguments) {
    to_freeze.push_back(element);
  }
  for (auto* function_frame : frame_stack) {
    for (auto* element : function_frame->elements) {
      if (element == nullptr) {
        continue;
      }
      to_freeze.push_back(element);
    }
  }
}

namespace {

starlark_obj* starlark_fn_trampoline(std::string_view fn_name, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  auto mod_info = state->loader->load_module(builtin_star_module);
  if (!mod_info.ok() || !(*mod_info)->ready()) {
    // This should never happen.
    error_callback.add_error(error_v2_unable_to_load_module(builtin_star_module));
    return nullptr;
  }
  for (std::size_t i = 0; i < (*mod_info)->get().first->elements.size(); ++i) {
    if ((*mod_info)->get().first->names->Get(i) == fn_name) {
      return (*mod_info)->get().first->elements[i]->call(pos_args, named_args, ctx, error_callback);
    }
  }
  // This should never happen.
  error_callback.add_error(error_v2_module_does_not_define_symbol(builtin_star_module, fn_name));
  return nullptr;
}

}  // namespace

starlark_obj* starlark_fn_max_impl(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return starlark_fn_trampoline("max", pos_args, named_args, ctx, error_callback);
}

starlark_obj* starlark_fn_min_impl(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return starlark_fn_trampoline("min", pos_args, named_args, ctx, error_callback);
}

starlark_obj* starlark_fn_sorted_impl(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return starlark_fn_trampoline("sorted", pos_args, named_args, ctx, error_callback);
}

}  // namespace interpreter
}  // namespace starlark

