// Copyright 2024-2025 Lucas Mirelmann

#ifndef CONTAINERS_LINKED_HASH_SET_HPP_
#define CONTAINERS_LINKED_HASH_SET_HPP_

#include <list>
#include <unordered_map>
#include <utility>

namespace starlark {
namespace cnt {

// A helper class for a hash map with predictable iteration order.
template<typename Key, typename Hash, typename KeyEqual>
class linked_hash_set {
 public:
  typedef Key value_type;

  linked_hash_set() {}

  size_t size() const {
    return values.size();
  }

  bool empty() const {
    return values.empty();
  }

  bool contains(const Key& key) const {
    return values.contains(key);
  }

  void clear() {
    values.clear();
    order.clear();
  }

  auto begin() {
    return order.begin();
  }

  auto end() {
    return order.end();
  }

  auto begin() const {
    return order.begin();
  }

  auto end() const {
    return order.end();
  }

  auto rbegin() {
    return order.rbegin();
  }

  auto rend() {
    return order.rend();
  }

  auto rbegin() const {
    return order.rbegin();
  }

  auto rend() const {
    return order.rend();
  }

  std::pair<typename std::list<Key>::const_iterator, bool> insert(const Key& value) {
    auto result = values.find(value);
    if (result != values.end()) {
      // If the value already exists, then keep the previous value. This is consistent with
      // how Python handles sets.
      return std::make_pair(result->second, false);
    }
    auto order_result = order.insert(order.end(), value);
    values[value] = order_result;
    return std::make_pair(order_result, true);
  }

  size_t erase(const Key& key) {
    auto result = values.find(key);
    if (result == values.end()) {
      return 0;
    }
    order.erase(result->second);
    values.erase(result);
    return 1;
  }

 private:
  // TODO(lmirelmann): This is designed for `Key` to be a pointer. 
  //   Either make this work without `Key` being a pointer, or as a contraint to the type
  // to only accept pointer types.
  std::list<Key> order;
  std::unordered_map<Key, typename std::list<Key>::iterator, Hash, KeyEqual> values;
};

}  // namespace cnt
}  // namespace starlark

#endif  // CONTAINERS_FLAT_MAP_HPP_

