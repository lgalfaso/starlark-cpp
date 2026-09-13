// Copyright 2026 Lucas Mirelmann

#ifndef STRING_LEVENSHTEIN_HPP_
#define STRING_LEVENSHTEIN_HPP_

#include <string>
#include <string_view>
#include <vector>

namespace starlark {
namespace string {

int levenshtein(std::string_view value, const std::vector<std::string>& candidates);

}  // namespace string
}  // namespace starlark

#endif  // STRING_LEVENSHTEIN_HPP_

