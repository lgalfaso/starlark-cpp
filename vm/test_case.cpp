// Copyright 2026 Lucas Mirelmann

#include "vm/test_case.hpp"

#include <iostream>
#include <string>

#include "vm/module_loader.hpp"

namespace starlark {
namespace vm {
namespace test {

std::map<std::string, std::string> split_test_case(std::string_view source) {
  std::string begin_module = "## Begin module";
  std::string end_module = "## End module";

  std::map<std::string, std::string> result;
  auto first_char = source.find_first_not_of("\n\r");
  if (first_char != std::string_view::npos) {
    source = source.substr(first_char);
  }
  while (source.starts_with(begin_module)) {
    auto begin_quote = source.find_first_of("\"'");
    if (begin_quote == std::string_view::npos) {
      std::cerr << "Invalid module\n";
      exit(1);
    }
    auto end_quote = source.find(source[begin_quote], begin_quote + 1);
    if (end_quote == std::string_view::npos) {
      std::cerr << "Invalid module name\n";
      exit(1);
    }
    auto it_end = source.find(end_module, end_quote);
    if (it_end == std::string_view::npos) {
      std::cerr << "Invalid module end\n";
      exit(1);
    }
    result[std::string{source.substr(begin_quote + 1, end_quote - begin_quote - 1)}] = source.substr(0, it_end + end_module.size());
    source = source.substr(it_end + end_module.size());
    first_char = source.find_first_not_of("\n\r");
    if (first_char != std::string_view::npos) {
      source = source.substr(first_char);
    }
  }
  result["main"] = source;
  return result;
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
