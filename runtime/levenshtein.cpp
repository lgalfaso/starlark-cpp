// Copyright 2026 Lucas Mirelmann

#include "runtime/levenshtein.hpp"

#include <algorithm>
#include <limits>
#include <numeric>
#include <string>
#include <string_view>
#include <vector>

namespace starlark {
namespace runtime {

int levenshtein(std::string_view value, const std::vector<std::string>& candidates) {
  int result  = -1;
  int64_t distance = std::numeric_limits<int64_t>::max();

  std::vector<int> buffer(value.size() + 1);
  for (int c = 0; c < candidates.size(); ++c) {
    const auto& candidate = candidates[c];

    std::iota(buffer.begin(), buffer.end(), 0);
    int previous;
    for (int i = 0; i < candidate.size(); ++i) {
      previous = buffer.front();
      buffer[0] = previous + 1;
      for (int j = 0; j < value.size(); ++j) {
        int match = candidate[i] == value[j] ? 0 : 1;
        int new_value = std::min(std::min(buffer[j] + 1, buffer[j + 1] + 1), previous + match);
        previous = buffer[j + 1];
        buffer[j + 1] = new_value;
      }
    }

    int max_possible = 1 + (candidate.size() + value.size()) / 3;
    if (buffer.back() < max_possible && buffer.back() < distance) {
      distance = buffer.back();
      result = c;
    }
  }
  return result;
}


}  // namespace runtime
}  // namespace starlark
