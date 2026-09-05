// Copyright 2026 Lucas Mirelmann

#ifndef VM_TEST_CASE_HPP_
#define VM_TEST_CASE_HPP_

#include <map>
#include <string>
#include <string_view>

#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {
namespace test {

std::map<std::string, std::string> split_test_case(std::string_view source);

starlark::vm::kv_module_loader make_test_loader(const std::map<std::string, std::string>& programs, const starlark::vm::module_info::bindings_t& custom_binding);

}  // namespace test
}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_TEST_CASE_HPP_
