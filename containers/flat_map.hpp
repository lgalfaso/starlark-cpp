// Copyright 2024 Lucas Mirelmann

#ifndef CONTAINER_FLAT_MAP_HPP_
#define CONTAINER_FLAT_MAP_HPP_

#include <vector>

namespace cnt {

// A helper class for sorted elements.
template<typename K, typename V>
class flat_map {
 public:
  flat_map(std::initializer_list<std::pair<K, V>> init) : values(init) {}

  std::vector<std::pair<K, V>>::const_iterator find(const K& key) const {
    auto candidate = std::lower_bound(values.begin(), values.end(), key,
        [](const std::pair<K, V>& kv, const K& k) {
            return kv.first < k;
        });
    if (candidate != end() && candidate->first == key) {
      return candidate;
    }
    return end();
  }

  std::vector<std::pair<K, V>>::const_iterator end() const {
    return values.end();
  }

 private:
  // Changing this class to use std::array and making this constexpr did not
  // show up any performance improvements.
  const std::vector<std::pair<K, V>> values;
};

}  // namespace cnt

#endif  // CONTAINER_FLAT_MAP_HPP_
