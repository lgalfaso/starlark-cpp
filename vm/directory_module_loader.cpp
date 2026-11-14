// Copyright 2026 Lucas Mirelmann

#include "vm/directory_module_loader.hpp"

#include <cassert>
#include <filesystem>  // NOLINT(build/c++17)
#include <optional>
#include <string>
#include <utility>

#include "io/read_file.hpp"
#include "status_or/status.hpp"
#include "vm/module_label.hpp"

using ::starlark::io::read_file;
using ::starlark::result::status_code;
using ::starlark::result::status_or;

namespace starlark {
namespace vm {

namespace {

const std::filesystem::path kParentDir("..");

// Lexically normalize (handles separators and `.` on all platforms), then reject parent hops.
std::optional<std::filesystem::path> normalized_relative_path(const std::filesystem::path& relative) {
  if (relative.empty() || relative.is_absolute()) {
    return std::nullopt;
  }
  const std::filesystem::path normalized = relative.lexically_normal();
  if (normalized.empty() || normalized.is_absolute() || normalized.has_root_path()) {
    return std::nullopt;
  }
  for (const auto& part : normalized) {
    if (part == kParentDir) {
      return std::nullopt;
    }
  }
  return normalized;
}

// Resolves root/relative without following symlinks; rejects any symlink in the path.
std::optional<std::filesystem::path> secure_path_under_root(const std::filesystem::path& root, std::string_view relative) {
  const auto relative_path = normalized_relative_path(std::filesystem::path(relative));
  if (!relative_path) {
    return std::nullopt;
  }

  std::filesystem::path current = root;
  for (auto it = relative_path->begin(); it != relative_path->end(); ++it) {
    current /= *it;
    const std::filesystem::file_status status = std::filesystem::symlink_status(current);
    if (status.type() == std::filesystem::file_type::not_found) {
      return std::nullopt;
    }
    if (std::filesystem::is_symlink(status)) {
      return std::nullopt;
    }
    const bool is_last = std::next(it) == relative_path->end();
    if (is_last) {
      if (!std::filesystem::is_regular_file(status)) {
        return std::nullopt;
      }
    } else if (!std::filesystem::is_directory(status)) {
      return std::nullopt;
    }
  }
  return current;
}

}  // namespace

directory_module_loader::directory_module_loader(std::filesystem::path root, std::string canonical_repo_name)
    : canonical_repo_name_(std::move(canonical_repo_name)) {
  assert(std::filesystem::is_directory(root));
  root_ = std::filesystem::weakly_canonical(std::move(root));
}

const module_info::bindings_t& directory_module_loader::bindings_for(std::string_view /*canonical_name*/) const {
  static const module_info::bindings_t kEmpty;
  return kEmpty;
}

fixed_bindings_directory_module_loader::fixed_bindings_directory_module_loader(std::filesystem::path root,
    std::string canonical_repo_name,
    module_info::bindings_t bindings)
    : directory_module_loader(std::move(root), std::move(canonical_repo_name)), bindings_(std::move(bindings)) {}

const module_info::bindings_t& fixed_bindings_directory_module_loader::bindings_for(
    std::string_view /*canonical_name*/) const {
  return bindings_;
}

const std::filesystem::path& directory_module_loader::root() const {
  return root_;
}

std::string directory_module_loader::resolve_canonical_name(std::string_view module_name, std::string_view caller_module_name) {
  const std::map<std::string, std::string, std::less<>> no_repo_mapping;
  const auto resolved = resolve_load_label(module_name, caller_module_name, no_repo_mapping);
  if (!resolved) {
    return std::string{module_name};
  }
  return *resolved;
}

status_or<module_source> directory_module_loader::resolve_source(std::string_view canonical_name_) {
  const auto parsed = parse_global_module_id(canonical_name_);
  if (!parsed || parsed->canonical_repo != canonical_repo_name_) {
    return status_or<module_source>(status_code::kStaticError);
  }
  const auto relative = label_to_relative_path(parsed->package, parsed->target);
  if (!relative) {
    return status_or<module_source>(status_code::kStaticError);
  }
  const auto file_path = secure_path_under_root(root_, *relative);
  if (!file_path) {
    return status_or<module_source>(status_code::kStaticError);
  }
  const auto file_contents = read_file(file_path->string());
  if (!file_contents) {
    return status_or<module_source>(status_code::kStaticError);
  }
  return status_or<module_source>(module_source{*file_contents, bindings_for(canonical_name_)});
}

}  // namespace vm
}  // namespace starlark
