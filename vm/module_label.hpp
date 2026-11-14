// Copyright 2026 Lucas Mirelmann

#ifndef VM_MODULE_LABEL_HPP_
#define VM_MODULE_LABEL_HPP_

#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#pragma GCC visibility push(default)

namespace starlark {
namespace vm {

struct parsed_label {
  std::string canonical_repo;
  bool repo_is_explicit = false;
  std::string package;
  std::string target;
};

// Parses global module IDs such as "@@//:builtin.star" or "@@rules_cc+1.0.3//pkg:file.bzl".
std::optional<parsed_label> parse_global_module_id(std::string_view global_id);

std::string format_global_module_id(std::string_view canonical_repo, std::string_view package, std::string_view target);

// Resolves a load() string using the caller's global module ID and the caller repo's apparent→canonical map.
std::optional<std::string> resolve_load_label(
    std::string_view load_string,
    std::string_view caller_global_id,
    const std::map<std::string, std::string, std::less<>>& apparent_to_canonical_repo);

// Maps a parsed label's package and target to a relative path under a repository root.
std::optional<std::string> label_to_relative_path(std::string_view package, std::string_view target);

}  // namespace vm
}  // namespace starlark

#pragma GCC visibility pop

#endif  // VM_MODULE_LABEL_HPP_
