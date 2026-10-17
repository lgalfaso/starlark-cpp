// Copyright 2026 Lucas Mirelmann

#include "vm/split_test_case.hpp"

#include <map>
#include <string>

namespace starlark {
namespace vm {

std::optional<std::map<std::string, std::string>> try_split_test_case(std::string_view source) {
  constexpr std::string_view begin_module = "## Begin module";
  constexpr std::string_view end_module = "## End module";

  std::map<std::string, std::string> result;
  auto first_char = source.find_first_not_of("\n\r");
  if (first_char != std::string_view::npos) {
    source = source.substr(first_char);
  }
  while (source.starts_with(begin_module)) {
    auto begin_quote = source.find_first_of("\"'");
    if (begin_quote == std::string_view::npos) {
      return std::nullopt;
    }
    auto end_quote = source.find(source[begin_quote], begin_quote + 1);
    if (end_quote == std::string_view::npos) {
      return std::nullopt;
    }
    auto it_end = source.find(end_module, end_quote);
    if (it_end == std::string_view::npos) {
      return std::nullopt;
    }
    result[std::string{source.substr(begin_quote + 1, end_quote - begin_quote - 1)}] =
        source.substr(0, it_end + end_module.size());
    source = source.substr(it_end + end_module.size());
    first_char = source.find_first_not_of("\n\r");
    if (first_char != std::string_view::npos) {
      source = source.substr(first_char);
    }
  }
  result["main"] = std::string(source);
  return result;
}

}  // namespace vm
}  // namespace starlark
