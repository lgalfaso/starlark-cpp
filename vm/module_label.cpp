// Copyright 2026 Lucas Mirelmann

#include "vm/module_label.hpp"

#include <string>
#include <utility>

namespace starlark {
namespace vm {

namespace {

bool is_valid_package_name(std::string_view package) {
  if (package.empty()) {
    return true;
  }
  if (package.front() == '/' || package.back() == '/') {
    return false;
  }
  if (package.find("//") != std::string_view::npos) {
    return false;
  }
  std::size_t start = 0;
  while (start <= package.size()) {
    const std::size_t next = package.find('/', start);
    const std::string_view segment = package.substr(start, next == std::string_view::npos ? package.size() - start : next - start);
    if (segment.empty() || segment == "." || segment == "..") {
      return false;
    }
    if (next == std::string_view::npos) {
      break;
    }
    start = next + 1;
  }
  return true;
}

std::string default_target_for_package(std::string_view package) {
  const std::size_t slash = package.rfind('/');
  if (slash == std::string_view::npos) {
    return std::string{package};
  }
  return std::string{package.substr(slash + 1)};
}

std::optional<std::pair<std::string, std::string>> parse_package_and_target(std::string_view label_tail) {
  if (label_tail.empty()) {
    return std::nullopt;
  }
  std::string package;
  std::string target;
  if (label_tail.starts_with(':')) {
    label_tail.remove_prefix(1);
    if (label_tail.empty()) {
      return std::nullopt;
    }
    package = "";
    target = std::string{label_tail};
  } else {
    const std::size_t colon = label_tail.find(':');
    if (colon == std::string_view::npos) {
      package = std::string{label_tail};
      if (!is_valid_package_name(package)) {
        return std::nullopt;
      }
      target = default_target_for_package(package);
    } else {
      if (colon + 1 >= label_tail.size()) {
        return std::nullopt;
      }
      package = std::string{label_tail.substr(0, colon)};
      if (!is_valid_package_name(package)) {
        return std::nullopt;
      }
      target = std::string{label_tail.substr(colon + 1)};
    }
  }
  if (target.empty()) {
    return std::nullopt;
  }
  return std::pair<std::string, std::string>{std::move(package), std::move(target)};
}

}  // namespace

std::optional<parsed_label> parse_global_module_id(std::string_view global_id) {
  if (!global_id.starts_with("@@")) {
    return std::nullopt;
  }
  global_id.remove_prefix(2);
  const std::size_t slash = global_id.find("//");
  if (slash == std::string_view::npos) {
    return std::nullopt;
  }
  parsed_label result{
      .canonical_repo = std::string{global_id.substr(0, slash)},
      .repo_is_explicit = true,
  };
  global_id.remove_prefix(slash + 2);
  const auto package_target = parse_package_and_target(global_id);
  if (!package_target) {
    return std::nullopt;
  }
  result.package = package_target->first;
  result.target = package_target->second;
  return result;
}

std::string format_global_module_id(std::string_view canonical_repo, std::string_view package, std::string_view target) {
  std::string result = "@@";
  result.append(canonical_repo);
  result.append("//");
  result.append(package);
  result.push_back(':');
  result.append(target);
  return result;
}

std::optional<std::string> resolve_load_label(
    std::string_view load_string,
    std::string_view caller_global_id,
    const std::map<std::string, std::string, std::less<>>& apparent_to_canonical_repo) {
  if (load_string.starts_with("@@")) {
    const auto parsed = parse_global_module_id(load_string);
    if (!parsed) {
      return std::nullopt;
    }
    return format_global_module_id(parsed->canonical_repo, parsed->package, parsed->target);
  }

  std::string caller_repo;
  std::string caller_package;
  if (caller_global_id.empty()) {
    caller_repo = "";
    caller_package = "";
  } else {
    const auto caller = parse_global_module_id(caller_global_id);
    if (!caller) {
      return std::nullopt;
    }
    caller_repo = caller->canonical_repo;
    caller_package = caller->package;
  }

  if (load_string.starts_with('@')) {
    load_string.remove_prefix(1);
    if (load_string.starts_with('@')) {
      load_string.remove_prefix(1);
      const std::size_t slash = load_string.find("//");
      if (slash == std::string_view::npos) {
        return std::nullopt;
      }
      const std::string canonical_repo{load_string.substr(0, slash)};
      load_string.remove_prefix(slash + 2);
      const auto package_target = parse_package_and_target(load_string);
      if (!package_target) {
        return std::nullopt;
      }
      return format_global_module_id(canonical_repo, package_target->first, package_target->second);
    }
    const std::size_t slash = load_string.find("//");
    if (slash == std::string_view::npos) {
      return std::nullopt;
    }
    const std::string apparent{load_string.substr(0, slash)};
    load_string.remove_prefix(slash + 2);
    const auto mapping = apparent_to_canonical_repo.find(apparent);
    if (mapping == apparent_to_canonical_repo.end()) {
      return std::nullopt;
    }
    const auto package_target = parse_package_and_target(load_string);
    if (!package_target) {
      return std::nullopt;
    }
    return format_global_module_id(mapping->second, package_target->first, package_target->second);
  }

  if (load_string.starts_with("//")) {
    load_string.remove_prefix(2);
    const auto package_target = parse_package_and_target(load_string);
    if (!package_target) {
      return std::nullopt;
    }
    return format_global_module_id(caller_repo, package_target->first, package_target->second);
  }

  if (load_string.starts_with(':')) {
    load_string.remove_prefix(1);
    if (load_string.empty()) {
      return std::nullopt;
    }
    return format_global_module_id(caller_repo, caller_package, load_string);
  }

  if (load_string.empty()) {
    return std::nullopt;
  }
  return format_global_module_id(caller_repo, caller_package, load_string);
}

std::optional<std::string> label_to_relative_path(std::string_view package, std::string_view target) {
  if (target.empty()) {
    return std::nullopt;
  }
  if (package.empty()) {
    return std::string{target};
  }
  std::string relative = std::string{package};
  relative.push_back('/');
  relative.append(target);
  return relative;
}

}  // namespace vm
}  // namespace starlark
