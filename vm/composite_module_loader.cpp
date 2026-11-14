// Copyright 2026 Lucas Mirelmann

#include "vm/composite_module_loader.hpp"

#include <cassert>
#include <string>
#include <utility>

#include "status_or/status.hpp"
#include "vm/module_label.hpp"

using ::starlark::result::status_code;
using ::starlark::result::status_or;

namespace starlark {
namespace vm {

void composite_module_loader::add_repository(std::string canonical_repo_name,
    std::unique_ptr<module_loader> loader,
    std::map<std::string, std::string, std::less<>> apparent_to_canonical_repo) {
  if (loader == nullptr) {
    assert(false && "composite_module_loader requires a non-null child loader");
    return;
  }
  if (repos_.contains(canonical_repo_name)) {
    assert(false && "duplicate canonical repository name in composite_module_loader");
    return;
  }
  repos_.emplace(std::move(canonical_repo_name),
      repo_entry{.loader = std::move(loader), .apparent_to_canonical = std::move(apparent_to_canonical_repo)});
}

const composite_module_loader::repo_entry* composite_module_loader::find_repo(std::string_view canonical_repo_name) const {
  const auto it = repos_.find(canonical_repo_name);
  if (it == repos_.end()) {
    return nullptr;
  }
  return &it->second;
}

const std::map<std::string, std::string, std::less<>>* composite_module_loader::mapping_for_caller(
    std::string_view caller_module_name) const {
  if (caller_module_name.empty()) {
    const repo_entry* main_repo = find_repo("");
    if (main_repo == nullptr) {
      return nullptr;
    }
    return &main_repo->apparent_to_canonical;
  }
  const auto caller = parse_global_module_id(caller_module_name);
  if (!caller) {
    return nullptr;
  }
  const repo_entry* entry = find_repo(caller->canonical_repo);
  if (entry == nullptr) {
    return nullptr;
  }
  return &entry->apparent_to_canonical;
}

std::string composite_module_loader::resolve_canonical_name(std::string_view module_name, std::string_view caller_module_name) {
  const std::map<std::string, std::string, std::less<>>* mapping = mapping_for_caller(caller_module_name);
  if (mapping == nullptr) {
    return std::string{module_name};
  }
  const auto resolved = resolve_load_label(module_name, caller_module_name, *mapping);
  if (!resolved) {
    return std::string{module_name};
  }
  return *resolved;
}

status_or<module_source> composite_module_loader::resolve_source(std::string_view canonical_name_) {
  const auto parsed = parse_global_module_id(canonical_name_);
  if (!parsed) {
    return status_or<module_source>(status_code::kStaticError);
  }
  const repo_entry* entry = find_repo(parsed->canonical_repo);
  if (entry == nullptr) {
    return status_or<module_source>(status_code::kStaticError);
  }
  return entry->loader->resolve_source(canonical_name_);
}

}  // namespace vm
}  // namespace starlark
