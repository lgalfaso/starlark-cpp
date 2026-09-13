// Copyright 2026 Lucas Mirelmann

#ifndef VM_FUNCTION_CALL_BINDING_HPP_
#define VM_FUNCTION_CALL_BINDING_HPP_

#include <map>
#include <span>
#include <string>
#include <vector>

#include "containers/linked_hash_map.hpp"
#include "google/protobuf/arena.h"
#include "runtime/starlark_object.hpp"
#include "vm/module_metadata.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

using kwargs_map_t = starlark::cnt::linked_hash_map<std::string, starlark::runtime::starlark_obj*, std::hash<std::string>, std::equal_to<std::string>>;

struct function_call_binding_state {
  starlark::runtime::starlark_obj::pos_args_t star_args;
  kwargs_map_t kwargs;
  std::vector<bool> filled_elements;
};

bool bind_function_arguments(const function_signature_metadata& fn_meta,
    const std::map<std::string, int, std::less<>>& named_argument_index,
    std::span<starlark::runtime::starlark_obj* const> default_arguments,
    std::vector<starlark::runtime::starlark_obj*>& frame_elements,
    const starlark::runtime::starlark_obj::pos_args_t& pos_args,
    const starlark::runtime::starlark_obj::named_args_t& named_args,
    function_call_binding_state& state,
    starlark::runtime::error_fn& error_callback);

void finish_bound_function_frame(const function_signature_metadata& fn_meta,
    google::protobuf::Arena& arena,
    std::vector<starlark::runtime::starlark_obj*>& frame_elements,
    const function_call_binding_state& binding_state,
    starlark::runtime::error_fn& error_callback);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_FUNCTION_CALL_BINDING_HPP_
