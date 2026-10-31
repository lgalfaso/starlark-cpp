// Copyright 2026 Lucas Mirelmann

#include "vm/function_call_binding.hpp"

#include <string_view>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "errors/runtime_error_messages.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_tuple.hpp"

namespace starlark {
namespace vm {

using starlark::runtime::context;
using starlark::runtime::error_fn;
using starlark::runtime::starlark_dictionary;
using starlark::runtime::starlark_obj;
using starlark::runtime::starlark_string;
using starlark::runtime::report_unexpected_keyword_argument;
using starlark::runtime::starlark_tuple;

bool bind_function_arguments(const function_signature_metadata& fn_meta,
    const std::map<std::string, int, std::less<>>& named_argument_index,
    std::span<starlark_obj* const> default_arguments,
    std::vector<starlark_obj*>& frame_elements,
    const starlark_obj::pos_args_t& pos_args,
    const starlark_obj::named_args_t& named_args,
    function_call_binding_state& state,
    error_fn& error_callback) {
  state.star_args.clear();
  state.kwargs.clear();
  state.filled_elements.assign(static_cast<std::size_t>(fn_meta.param_count), false);

  int next_positional_param = 0;
  const int number_positional_params = fn_meta.positional_param_count;
  for (auto* param : pos_args) {
    if (next_positional_param < number_positional_params) {
      const auto& signature_param = fn_meta.params[static_cast<std::size_t>(next_positional_param)];
      frame_elements[signature_param.pos_in_frame] = param;
      state.filled_elements[static_cast<std::size_t>(next_positional_param)] = true;
      next_positional_param++;
    } else if (fn_meta.has_star_argument) {
      state.star_args.push_back(param);
    } else {
      error_callback.add_error(starlark::error_messages::error_v2_arguments_exactly(
          fn_meta.fn_name, pos_args.size(), number_positional_params));
      return false;
    }
  }

  for (auto& kwparam : named_args) {
    auto it = named_argument_index.find(std::string{kwparam.first->as_string()});
    if (it == named_argument_index.end()) {
      if (fn_meta.has_star_star_argument) {
        state.kwargs.insert(std::string{kwparam.first->as_string()}, kwparam.second);
      } else {
        std::vector<std::string_view> parameter_names;
        parameter_names.reserve(fn_meta.params.size());
        for (const auto& param : fn_meta.params) {
          parameter_names.emplace_back(param.name);
        }
        report_unexpected_keyword_argument(
            error_callback, fn_meta.fn_name, kwparam.first->as_string(), parameter_names);
        return false;
      }
      continue;
    }
    const auto pos = it->second;
    if (state.filled_elements[static_cast<std::size_t>(pos)]) {
      error_callback.add_error(starlark::error_messages::error_v2_multiple_values_for_argument(
          fn_meta.fn_name, kwparam.first->as_string()));
      return false;
    }
    frame_elements[fn_meta.params[static_cast<std::size_t>(pos)].pos_in_frame] = kwparam.second;
    state.filled_elements[static_cast<std::size_t>(pos)] = true;
  }

  int default_argument_pos = 0;
  for (int i = 0; i < number_positional_params; ++i) {
    if (!state.filled_elements[static_cast<std::size_t>(i)]) {
      if (fn_meta.params[static_cast<std::size_t>(i)].default_initialization) {
        frame_elements[fn_meta.params[static_cast<std::size_t>(i)].pos_in_frame] =
            default_arguments[static_cast<std::size_t>(default_argument_pos)];
      } else {
        error_callback.add_error(starlark::error_messages::error_v2_missing_positional_argument(
            fn_meta.fn_name, fn_meta.params[static_cast<std::size_t>(i)].name));
        return false;
      }
    }
    if (fn_meta.params[static_cast<std::size_t>(i)].default_initialization) {
      default_argument_pos++;
    }
  }

  auto keyword_only_parameter_start = number_positional_params;
  if (fn_meta.has_star_argument) {
    keyword_only_parameter_start++;
  }
  for (int i = keyword_only_parameter_start; i < keyword_only_parameter_start + fn_meta.keyword_only_parameter_count; ++i) {
    if (!state.filled_elements[static_cast<std::size_t>(i)]) {
      if (fn_meta.params[static_cast<std::size_t>(i)].default_initialization) {
        frame_elements[fn_meta.params[static_cast<std::size_t>(i)].pos_in_frame] =
            default_arguments[static_cast<std::size_t>(default_argument_pos)];
      } else {
        error_callback.add_error(starlark::error_messages::error_v2_missing_keyword_only_argument(
            fn_meta.fn_name, fn_meta.params[static_cast<std::size_t>(i)].name));
        return false;
      }
    }
    if (fn_meta.params[static_cast<std::size_t>(i)].default_initialization) {
      default_argument_pos++;
    }
  }

  return true;
}

void finish_bound_function_frame(const function_signature_metadata& fn_meta,
    context& ctx,
    std::vector<starlark_obj*>& frame_elements,
    const function_call_binding_state& binding_state,
    error_fn& error_callback) {
  if (fn_meta.has_star_argument) {
    auto* tuple = ctx.create<starlark_tuple>(binding_state.star_args.size());
    for (auto* element : binding_state.star_args) {
      tuple->add(element);
    }
    frame_elements[fn_meta.params[static_cast<std::size_t>(fn_meta.positional_param_count)].pos_in_frame] = tuple;
  }
  if (fn_meta.has_star_star_argument) {
    auto* dict = ctx.create<starlark_dictionary>();
    for (const auto& [name, value] : binding_state.kwargs) {
      dict->insert(ctx.create<starlark_string>(name), value, error_callback);
    }
    frame_elements[fn_meta.params[static_cast<std::size_t>(fn_meta.param_count - 1)].pos_in_frame] = dict;
  }
}

}  // namespace vm
}  // namespace starlark
