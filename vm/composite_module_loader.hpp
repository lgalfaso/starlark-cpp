// Copyright 2026 Lucas Mirelmann

#ifndef VM_COMPOSITE_MODULE_LOADER_HPP_
#define VM_COMPOSITE_MODULE_LOADER_HPP_

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

class composite_module_loader : public module_loader {
 public:
  void add_repository(std::string canonical_repo_name,
      std::unique_ptr<module_loader> loader,
      std::map<std::string, std::string, std::less<>> apparent_to_canonical_repo);

  std::string resolve_canonical_name(std::string_view module_name, std::string_view caller_module_name) override;
  starlark::result::status_or<module_source> resolve_source(std::string_view canonical_name_) override;

 private:
  struct repo_entry {
    std::unique_ptr<module_loader> loader;
    std::map<std::string, std::string, std::less<>> apparent_to_canonical;
  };

  const repo_entry* find_repo(std::string_view canonical_repo_name) const;
  const std::map<std::string, std::string, std::less<>>* mapping_for_caller(std::string_view caller_module_name) const;

  std::map<std::string, repo_entry, std::less<>> repos_;
};

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_COMPOSITE_MODULE_LOADER_HPP_
