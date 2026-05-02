// Copyright 2026 Lucas Mirelmann

#include "interpreter/function.hpp"

#include <format>
#include <string>
#include <utility>
#include <vector>

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
    std::string_view module_name,
    const google::protobuf::RepeatedPtrField<std::string>* frame_names,
    std::vector<std::vector<frame*>>& frame_stacks,
    std::vector<std::pair<int, int>>& call_stack,
    int& instruction_ptr,
    int& block_ptr) :
      starlark::runtime::starlark_function(function_signature->fn_name(), module_name),
      entrypoint(entrypoint),
      default_arguments(default_arguments),
      function_signature(function_signature),
      frame_names(frame_names),
      frame_stack(frame_stacks.back()),
      frame_stacks(frame_stacks),
      call_stack(call_stack),
      instruction_ptr(instruction_ptr),
      block_ptr(block_ptr) {
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
    new_frame->elements[pos] = kwparam.second;
    filled_elements[pos] = true;
  }
  // Put the default arguments, and check that all the slots are filled.
  int default_argument_pos = 0;
  for (int i = 0; i < number_of_standard_params; ++i) {
    if (!filled_elements[i]) {
      if (function_signature->param(i).default_initialization()) {
        new_frame->elements[i] = default_arguments[default_argument_pos];
      } else {
        if (i < number_positional_params) {
          error_callback.add_error(error_missing_positional_argument(
              function_signature->fn_name(),
              function_signature->param(i).name()));
        } else {
          error_callback.add_error(error_missing_keyword_only_argument(
              function_signature->fn_name(),
              function_signature->param(i).name()));
        }
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
    new_frame->elements[number_of_standard_params] = tuple;
  }
  // Fill **kwargs.
  if (function_signature->has_star_star_argument()) {
    auto* dict = Arena::Create<starlark_dictionary>(&ctx.arena());
    for (auto& element : kwargs) {
      dict->insert(Arena::Create<starlark_string>(&ctx.arena(), element.first), element.second, error_callback);
    }
    new_frame->elements[function_signature->param().size() - 1] = dict;
  }

  frame_stacks.push_back(frame_stack);
  frame_stacks.back().push_back(new_frame);
  call_stack.push_back(std::make_pair(block_ptr, instruction_ptr));
  block_ptr = entrypoint;
  instruction_ptr = 0;
  return ctx.none_value();
}

bool interpreter_function::inner_equals(starlark::runtime::equals_comparator& comp, const starlark_obj* other) const {
  // TODO(lmirelmann): Implement.
  return false;
}

}  // namespace interpreter
}  // namespace starlark

