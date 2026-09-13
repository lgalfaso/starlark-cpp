// Copyright 2026 Lucas Mirelmann

#include "vm/module_load_tracker.hpp"

namespace starlark {
namespace vm {

std::string report_recursion_in_modules(const std::vector<std::string>& module_lookup, std::size_t initial_pos) {
  std::string result;
  for (std::size_t i = 0; i < initial_pos; ++i) {
    result += "    ";
    result += module_lookup[i];
    result += "\n";
  }
  result += "+-> ";
  result += module_lookup[initial_pos];
  result += "\n";
  for (std::size_t i = initial_pos + 1; i < module_lookup.size(); ++i) {
    result += "|   ";
    result += module_lookup[i];
    result += "\n";
  }
  result += "+-> ";
  result += module_lookup[initial_pos];
  result += "\n";
  return result;
}

}  // namespace vm
}  // namespace starlark
