// Copyright 2026 Lucas Mirelmann

#ifndef VM_MODULE_LOAD_TRACKER_HPP_
#define VM_MODULE_LOAD_TRACKER_HPP_

#include <cstddef>

#include <map>
#include <string>
#include <vector>

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

std::string report_recursion_in_modules(const std::vector<std::string>& module_lookup, std::size_t initial_pos);

struct module_load_tracker {
  std::vector<std::string> module_lookup;
  std::map<std::string, std::size_t, std::less<>> module_processing;
  bool module_reduction = false;
};

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_MODULE_LOAD_TRACKER_HPP_
