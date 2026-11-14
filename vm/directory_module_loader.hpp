// Copyright 2026 Lucas Mirelmann

#ifndef VM_DIRECTORY_MODULE_LOADER_HPP_
#define VM_DIRECTORY_MODULE_LOADER_HPP_

#include <filesystem>  // NOLINT(build/c++17)
#include <string>

#include "vm/module_loader.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

class directory_module_loader : public module_loader {
 public:
  directory_module_loader(std::filesystem::path root, std::string canonical_repo_name = "");

  const std::filesystem::path& root() const;

  std::string resolve_canonical_name(std::string_view module_name, std::string_view caller_module_name) override;
  starlark::result::status_or<module_source> resolve_source(std::string_view canonical_name_) override;

 protected:
  virtual const module_info::bindings_t& bindings_for(std::string_view canonical_name) const;

  std::filesystem::path root_;
  std::string canonical_repo_name_;
};

// Same custom bindings for every module in the repository.
class fixed_bindings_directory_module_loader : public directory_module_loader {
 public:
  fixed_bindings_directory_module_loader(std::filesystem::path root,
      std::string canonical_repo_name = "",
      module_info::bindings_t bindings = {});

 protected:
  const module_info::bindings_t& bindings_for(std::string_view canonical_name) const override;

 private:
  module_info::bindings_t bindings_;
};

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_DIRECTORY_MODULE_LOADER_HPP_
