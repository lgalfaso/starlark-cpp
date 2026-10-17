// Copyright 2026 Lucas Mirelmann

#include "vm/test_case.hpp"

#include <iostream>
#include <string>
#include <functional>
#include <map>
#include <utility>

#include "vm/module_loader.hpp"
#include "vm/split_test_case.hpp"

namespace starlark {
namespace vm {
namespace test {

std::map<std::string, std::string> split_test_case(std::string_view source) {
  auto parsed = starlark::vm::try_split_test_case(source);
  if (!parsed.has_value()) {
    std::cerr << "Invalid module test case\n";
    exit(1);
  }
  return *parsed;
}

kv_module_loader make_test_loader(const std::map<std::string, std::string>& programs, const starlark::vm::module_info::bindings_t& custom_binding) {
  std::map<std::string, std::pair<std::string, const starlark::vm::module_info::bindings_t>, std::less<>> modules;
  for (const auto& [name, source] : programs) {
    modules.try_emplace(name, source, custom_binding);
  }
  return kv_module_loader{modules};
}

}  // namespace test
}  // namespace vm
}  // namespace starlark
