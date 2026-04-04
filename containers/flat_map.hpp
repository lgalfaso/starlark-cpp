// Copyright 2024-2025 Lucas Mirelmann

#ifndef CONTAINERS_FLAT_MAP_HPP_
#define CONTAINERS_FLAT_MAP_HPP_

#include <algorithm>
#include <array>
#include <utility>

namespace starlark {
namespace cnt {

template<typename K, typename V, std::size_t N>
class flat_map {
 public:
  constexpr flat_map(std::initializer_list<std::pair<K, V>> init) : values(make_array(init)) {}

  static constexpr std::array<std::pair<K, V>, N>
  make_array(std::initializer_list<std::pair<K, V>> init) {
    std::array<std::pair<K, V>, N> result{};

    std::size_t i = 0;
    for (auto v : init) {
      result[i++] = v;
    }
    return result;
  }

  std::array<std::pair<K, V>, N>::const_iterator find(const K& key) const {
    auto candidate = std::lower_bound(values.begin(), values.end(), key,
        [](const std::pair<K, V>& kv, const K& k) {
            return kv.first < k;
        });
    if (candidate != end() && candidate->first == key) {
      return candidate;
    }
    return end();
  }

  std::array<std::pair<K, V>, N>::const_iterator upper_bound(const K& key) const {
    return std::upper_bound(values.begin(), values.end(), key,
        [](const K& k, const std::pair<K, V>& kv) -> bool {
            return k < kv.first;
        });
  }

  std::array<std::pair<K, V>, N>::const_iterator begin() const {
    return values.begin();
  }

  std::array<std::pair<K, V>, N>::const_iterator end() const {
    return values.end();
  }

 private:
  const std::array<std::pair<K, V>, N> values;
};

}  // namespace cnt
}  // namespace starlark

#endif  // CONTAINERS_FLAT_MAP_HPP_
