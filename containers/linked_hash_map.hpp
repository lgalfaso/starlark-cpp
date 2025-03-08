// Copyright 2024-2025 Lucas Mirelmann

#ifndef CONTAINERS_LINKED_HASH_MAP_HPP_
#define CONTAINERS_LINKED_HASH_MAP_HPP_

#include <list>
#include <unordered_map>
#include <utility>

namespace starlark {
namespace cnt {

// A helper class for a hash map with predictable iteration order.
template<typename Key, typename Value, typename Hash, typename KeyEqual>
class linked_hash_map {
 public:
  typedef std::pair<const Key, Value> value_type;

  linked_hash_map() {}

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

  std::pair<typename std::list<std::pair<const Key, Value>>::iterator, bool> insert(const Key& key, const Value& value) {
    auto result = values.find(key);
    if (result != values.end()) {
      // If the value already exists, then replace the value.
      result->second->second = value;
      return std::make_pair(result->second, false);
    }
    auto order_result = order.insert(order.end(), std::make_pair(key, value));
    values[key] = order_result;
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
  // TODO(lmirelmann): This is designed for `Key` and `Value` to be pointers. 
  //   Either make this work without `Key` and `Value` being pointers, or as a contraint to the type
  // to only accept pointer types.
  // One simple way to do this is to make the unordered container to use pointers as the values.
  std::list<std::pair<const Key, Value>> order;
  std::unordered_map<Key, typename std::list<std::pair<const Key, Value>>::iterator, Hash, KeyEqual> values;
};

}  // namespace cnt
}  // namespace starlark

#endif  // CONTAINERS_FLAT_MAP_HPP_

