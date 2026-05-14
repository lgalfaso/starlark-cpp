// Copyright 2026 Lucas Mirelmann

#include "interpreter/function.hpp"

#include <format>
#include <string>
#include <utility>
#include <vector>

#include "interpreter/runner_state.hpp"
#include "runtime/error_messages.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::runtime::context;
using ::starlark::runtime::error_arguments_exactly;
using ::starlark::runtime::error_fn;
using ::starlark::runtime::error_missing_keyword_only_argument;
using ::starlark::runtime::error_missing_positional_argument;
using ::starlark::runtime::error_multiple_values_for_argument;
using ::starlark::runtime::error_unexpected_keyword_argument;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;

namespace starlark {
namespace interpreter {

interpreter_function::interpreter_function(
    int entrypoint,
    std::vector<starlark::runtime::starlark_obj*>&& default_arguments,
    const starlark::bytecode::FunctionSignature* function_signature,
    starlark::bytecode::Program* program,
    std::string_view module_name,
    const google::protobuf::RepeatedPtrField<std::string>* frame_names,
    const std::vector<frame*>& frame_stack) :
      starlark::runtime::starlark_function(function_signature->fn_name(), module_name),
      entrypoint(entrypoint),
      default_arguments(default_arguments),
      function_signature(function_signature),
      frame_names(frame_names),
      frame_stack(frame_stack),
      current_program(program, module_name) {
  default_parameters = nullptr;
  for (std::size_t i = 0; i < function_signature->param().size(); ++i) {
    named_argument_index[function_signature->param(i).name()] = i;
  }
}

starlark_obj* interpreter_function::call(
      const starlark_obj::pos_args_t& pos_args,
      const starlark_obj::named_args_t& named_args,
      context& ctx,
      error_fn& error_callback) {
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  // TODO(lmirelmann): This is an O(n) operation, this should be improved if it becomes an issue.
  if (!ctx.options().allow_recursion) {
    for (const auto* other : state->call_fns) {
      if (equals(*other)) {
        error_callback.add_error(std::format("Error: function '{}' called recursively", fn_name));
        return nullptr;
      }
    }
  }
  state->call_fns.push_back(this);
  auto* new_frame = Arena::Create<frame>(&ctx.arena(), frame_names);
  starlark_obj::pos_args_t args;
  starlark_obj::named_args_t kwargs;
  int next_positional_param = 0;
  int number_of_standard_params = function_signature->param().size();
  if (function_signature->has_star_argument()) {
    number_of_standard_params--;
  }
  if (function_signature->has_star_star_argument()) {
    number_of_standard_params--;
  }
  int number_positional_params = number_of_standard_params - function_signature->keyword_only_parameter_count();

  std::vector<bool> filled_elements(frame_names->size());
  // Process the positional arguments.
  for (auto* param : pos_args) {
    if (next_positional_param < number_positional_params) {
      auto& signature_param = function_signature->param(next_positional_param);
      new_frame->elements[signature_param.pos().pos_in_frame()] = param;
      filled_elements[next_positional_param] = true;
      next_positional_param++;
    } else if (function_signature->has_star_argument()) {
      args.push_back(param);
    } else {
      error_callback.add_error(error_arguments_exactly(
          function_signature->fn_name(),
          pos_args.size(),
          number_positional_params));
      return nullptr;
    }
  }
  // Process the named arguments.
  for (auto& kwparam : named_args) {
    auto it = named_argument_index.find(kwparam.first);
    if (it == named_argument_index.end()) {
      if (function_signature->has_star_star_argument()) {
        kwargs.insert(kwparam.first, kwparam.second);
      } else {
        error_callback.add_error(error_unexpected_keyword_argument(
            function_signature->fn_name(),
            kwparam.first));
        return nullptr;
      }
      continue;
    }
    auto pos = it->second;
    if (filled_elements[pos]) {
      error_callback.add_error(error_multiple_values_for_argument(
          function_signature->fn_name(),
          kwparam.first));
      return nullptr;
    }
    new_frame->elements[function_signature->param(pos).pos().pos_in_frame()] = kwparam.second;
    filled_elements[pos] = true;
  }
  // Put the default arguments, and check that all the slots are filled on positional arguments.
  int default_argument_pos = 0;
  for (int i = 0; i < number_positional_params; ++i) {
    if (!filled_elements[i]) {
      if (function_signature->param(i).default_initialization()) {
        new_frame->elements[function_signature->param(i).pos().pos_in_frame()] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_missing_positional_argument(
            function_signature->fn_name(),
            function_signature->param(i).name()));
        return nullptr;
      }
    }
    if (function_signature->param(i).default_initialization()) {
      default_argument_pos++;
    }
  }
  // Put the default arguments, and check that all the slots are filled on keyword-only arguments.
  auto keyword_only_parameter_start = number_positional_params;
  if (function_signature->has_star_argument()) {
    keyword_only_parameter_start++;
  }
  for (int i = keyword_only_parameter_start; i < keyword_only_parameter_start + function_signature->keyword_only_parameter_count(); ++i) {
    if (!filled_elements[i]) {
      if (function_signature->param(i).default_initialization()) {
        new_frame->elements[function_signature->param(i).pos().pos_in_frame()] = default_arguments[default_argument_pos];
      } else {
        error_callback.add_error(error_missing_keyword_only_argument(
            function_signature->fn_name(),
            function_signature->param(i).name()));
        return nullptr;
      }
    }
    if (function_signature->param(i).default_initialization()) {
      default_argument_pos++;
    }
  }
  // Fill *args.
  if (function_signature->has_star_argument()) {
    auto* tuple = Arena::Create<starlark_tuple>(&ctx.arena(), args.size());
    for (auto* element : args) {
      tuple->add(element);
    }
    new_frame->elements[function_signature->param(number_positional_params).pos().pos_in_frame()] = tuple;
  }
  // Fill **kwargs.
  if (function_signature->has_star_star_argument()) {
    auto* dict = Arena::Create<starlark_dictionary>(&ctx.arena());
    for (auto& element : kwargs) {
      dict->insert(Arena::Create<starlark_string>(&ctx.arena(), element.first), element.second, error_callback);
    }
    new_frame->elements[function_signature->param(function_signature->param().size() - 1).pos().pos_in_frame()] = dict;
  }

  state->frame_stacks.push_back(frame_stack);
  state->frame_stacks.back().push_back(new_frame);
  state->call_stack.push_back(std::make_pair(state->block_ptr, state->instruction_ptr));
  state->block_ptr = entrypoint;
  state->instruction_ptr = 0;
  state->current_program_stack.push_back(state->current_program);
  state->current_program = &current_program;
  return ctx.none_value();
}

bool interpreter_function::inner_equals(starlark::runtime::equals_comparator& comp, const starlark_obj* other) const {
  if (other->type() != type()) {
    return false;
  }
  const interpreter_function* f_other = reinterpret_cast<const interpreter_function*>(other);
  if (entrypoint != f_other->entrypoint) {
    return false;
  }
  if (current_program.first != f_other->current_program.first) {
    return false;
  }
  if (frame_stack != f_other->frame_stack) {
    return false;
  }
  return true;
}

void interpreter_function::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  for (auto* element : default_arguments) {
    to_freeze.push_back(element);
  }
}

namespace {

starlark_obj* starlark_fn_trampoline(std::string_view fn_name, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  runner_state* state = static_cast<runner_state*>(ctx.runner_context());
  auto mod_info = state->loader->load_module(builtin_star_module, "");
  if (!mod_info.ok() || !(*mod_info)->ready()) {
    // This should never happen.
    error_callback.add_error(std::format("ModuleNotFoundError: Unable to load module named '{}'", builtin_star_module));
    return nullptr;
  }
  for (std::size_t i = 0; i < (*mod_info)->get().first->elements.size(); ++i) {
    if ((*mod_info)->get().first->names->Get(i) == fn_name) {
      return (*mod_info)->get().first->elements[i]->call(pos_args, named_args, ctx, error_callback);
    }
  }
  // This should never happen.
  error_callback.add_error(std::format("LoadError: Module '{}' does not contain the symbol {}", builtin_star_module, fn_name));
  return nullptr;
}

}  // namespace

starlark_obj* starlark_fn_max_impl(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return starlark_fn_trampoline("max_impl", pos_args, named_args, ctx, error_callback);
}

starlark_obj* starlark_fn_min_impl(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return starlark_fn_trampoline("min_impl", pos_args, named_args, ctx, error_callback);
}

starlark_obj* starlark_fn_sorted_impl(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) {
  return starlark_fn_trampoline("sorted_impl", pos_args, named_args, ctx, error_callback);
}

}  // namespace interpreter
}  // namespace starlark

