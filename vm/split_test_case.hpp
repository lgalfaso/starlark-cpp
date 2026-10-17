// Copyright 2026 Lucas Mirelmann

#ifndef VM_SPLIT_TEST_CASE_HPP_
#define VM_SPLIT_TEST_CASE_HPP_

#include <map>
#include <optional>
#include <string>
#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

// Parses multi-module test files (`## Begin module "name"` … `## End module`).
// Returns nullopt when the prefix is present but malformed.
std::optional<std::map<std::string, std::string>> try_split_test_case(std::string_view source);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_SPLIT_TEST_CASE_HPP_
