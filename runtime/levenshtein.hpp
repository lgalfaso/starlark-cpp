// Copyright 2026 Lucas Mirelmann

#ifndef RUNTIME_LEVENSHTEIN_HPP_
#define RUNTIME_LEVENSHTEIN_HPP_

#include <string>
#include <string_view>
#include <vector>

namespace starlark {
namespace runtime {

int levenshtein(std::string_view value, const std::vector<std::string>& candidates);

}  // namespace runtime
}  // namespace starlark



#endif  // RUNTIME_LEVENSHTEIN_HPP_

