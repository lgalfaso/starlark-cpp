// Copyright 2026 Lucas Mirelmann

#include "interpreter/function.hpp"

#include <format>
#include <string>
#include <utility>
#include <vector>

#include "interpreter/runner_state.hpp"
#include "string/levenshtein.hpp"
#include "errors/runtime_error_messages.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"

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
    const google::protobuf::RepeatedPtrField<std::string>* frame_names,
    const std::vector<frame*>& frame_stack) :
      starlark::runtime::starlark_function(current_program_.bytecode->block(entrypoint).function_signature().fn_name(), current_program_.module_name),
      entrypoint(entrypoint),
      default_arguments(default_arguments),
      frame_names(frame_names),
      frame_stack(frame_stack),
      current_program(current_program_),
      inner_(inner_fn) {
  default_parameters = nullptr;
  for (std::size_t i = 0; i < current_program_.bytecode->block(entrypoint).function_signature().param().size(); ++i) {
    named_argument_index[current_program_.bytecode->block(entrypoint).function_signature().param(i).name()] = i;
  }
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
  starlark_obj::named_args_t kwargs;
  int next_positional_param = 0;
  const auto& function_signature = current_program.bytecode->block(entrypoint).function_signature();
  int number_of_standard_params = function_signature.param().size();
  if (function_signature.has_star_argument()) {
    number_of_standard_params--;
  }
  if (function_signature.has_star_star_argument()) {
    number_of_standard_params--;
  }
  int number_positional_params = number_of_standard_params - function_signature.keyword_only_parameter_count();

  std::vector<bool> filled_elements(frame_names->size());
  // Process the positional arguments.
  for (auto* param : pos_args) {
    if (next_positional_param < number_positional_params) {
      auto& signature_param = function_signature.param(next_positional_param);
      new_frame->elements[signature_param.pos().pos_in_frame()] = param;
      filled_elements[next_positional_param] = true;
      next_positional_param++;
    } else if (function_signature.has_star_argument()) {
      args.push_back(param);
    } else {
      error_callback.add_error(error_v2_arguments_exactly(
          function_signature.fn_name(),
          pos_args.size(),
          number_positional_params));
      return nullptr;
    }
  }
  // Process the named arguments.
  for (auto& kwparam : named_args) {
    auto it = named_argument_index.find(kwparam.first);
    if (it == named_argument_index.end()) {
      if (function_signature.has_star_star_argument()) {
        kwargs.insert(kwparam.first, kwparam.second);
      } else {
        std::vector<std::string> all_candidates;
        for (const auto& param : function_signature.param()) {
          all_candidates.emplace_back(param.name());
        }
        auto candidate = starlark::string::levenshtein(kwparam.first, all_candidates);
        if (candidate >= 0) {
          error_callback.add_error(error_v2_unexpected_keyword_argument_with_hint(
              function_signature.fn_name(),
              kwparam.first,
              all_candidates[candidate]), all_candidates[candidate]);
        } else {
          error_callback.add_error(error_v2_unexpected_keyword_argument(
              function_signature.fn_name(),
              kwparam.first));
        }
        return nullptr;
      }
      continue;
    }
    auto pos = it->second;
    if (filled_elements[pos]) {
      error_callback.add_error(error_v2_multiple_values_for_argument(
          function_signature.fn_name(),
          kwparam.first));
      return nullptr;
    }
    new_frame->elements[function_signature.param(pos).pos().pos_in_frame()] = kwparam.second;
    filled_elements[pos] = true;
  }
  // Put the default arguments, and check that all the slots are filled on positional arguments.
  int default_argument_pos = 0;
  for (int i = 0; i < number_positional_params; ++i) {
    if (!filled_elements[i]) {
      if (function_signature.param(i).default_initialization()) {
        new_frame->elements[function_signature.param(i).pos().pos_in_frame()] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_v2_missing_positional_argument(
            function_signature.fn_name(),
            function_signature.param(i).name()));
        return nullptr;
      }
    }
    if (function_signature.param(i).default_initialization()) {
      default_argument_pos++;
    }
  }
  // Put the default arguments, and check that all the slots are filled on keyword-only arguments.
  auto keyword_only_parameter_start = number_positional_params;
  if (function_signature.has_star_argument()) {
    keyword_only_parameter_start++;
  }
  for (int i = keyword_only_parameter_start; i < keyword_only_parameter_start + function_signature.keyword_only_parameter_count(); ++i) {
    if (!filled_elements[i]) {
      if (function_signature.param(i).default_initialization()) {
        new_frame->elements[function_signature.param(i).pos().pos_in_frame()] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_v2_missing_keyword_only_argument(
            function_signature.fn_name(),
            function_signature.param(i).name()));
        return nullptr;
      }
    }
    if (function_signature.param(i).default_initialization()) {
      default_argument_pos++;
    }
  }
  // Fill *args.
  if (function_signature.has_star_argument()) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), args.size());
    for (auto* element : args) {
      tuple->add(element);
    }
    new_frame->elements[function_signature.param(number_positional_params).pos().pos_in_frame()] = tuple;
  }
  // Fill **kwargs.
  if (function_signature.has_star_star_argument()) {
    auto* dict = Arena::Create<starlark_dictionary>(&ctx.arena());
    for (auto& element : kwargs) {
      dict->insert(Arena::Create<starlark_string>(&ctx.arena(), element.first), element.second, error_callback);
    }
    new_frame->elements[function_signature.param(function_signature.param().size() - 1).pos().pos_in_frame()] = dict;
  }

  state->frame_stacks.push_back(frame_stack);
  state->frame_stacks.back().push_back(new_frame);
  state->call_stack.push_back(call_stack_entry{.block_ptr = state->block_ptr, .instruction_ptr = state->instruction_ptr, .inner = state->inner});
  state->block_ptr = entrypoint;
  state->instruction_ptr = 0;
  state->inner = inner_;
  state->current_program_stack.push_back(state->current_program);
  state->current_program = current_program;
  return ctx.none_value();
}

bool interpreter_function::inner_equals(starlark::runtime::equals_comparator& comp, const starlark_obj* other) const {
  if (other->type() != type()) {
    return false;
  }
  const interpreter_function* f_other = reinterpret_cast<const interpreter_function*>(other);
  return cmp_fn(this, f_other, true) == 0;
}

void interpreter_function::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, bool extended, error_fn& error_callback) const {
  if (extended && type() == other->type() && equals(*other)) {
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

