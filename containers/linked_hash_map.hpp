// Copyright 2024-2025 Lucas Mirelmann

#ifndef CONTAINERS_LINKED_HASH_MAP_HPP_
#define CONTAINERS_LINKED_HASH_MAP_HPP_

#include <functional>
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
  typedef std::list<std::pair<const Key, Value>>::iterator iterator;
  typedef std::list<std::pair<const Key, Value>>::const_iterator const_iterator;
  typedef std::list<std::pair<const Key, Value>>::reverse_iterator reverse_iterator;
  typedef std::list<std::pair<const Key, Value>>::const_reverse_iterator const_reverse_iterator;

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

  iterator begin() {
    return order.begin();
  }

  iterator end() {
    return order.end();
  }

  const_iterator begin() const {
    return order.begin();
  }

  const_iterator end() const {
    return order.end();
  }

  reverse_iterator rbegin() {
    return order.rbegin();
  }

  reverse_iterator rend() {
    return order.rend();
  }

  const_reverse_iterator rbegin() const {
    return order.rbegin();
  }

  const_reverse_iterator rend() const {
    return order.rend();
  }

  std::list<std::pair<const Key, Value>>::const_iterator find(const Key& key) const {
    auto inner_result = values.find(key);
    if (inner_result == values.end()) {
      return order.cend();
    }
    return inner_result->second;
  }

  std::pair<typename std::list<std::pair<const Key, Value>>::iterator, bool> insert(const Key& key, const Value& value) {
    auto result = values.find(key);
    if (result != values.end()) {
      // If the value already exists, then replace the value.
      result->second->second = value;
      return std::make_pair(result->second, false);
    }
    auto order_result = order.insert(order.end(), std::make_pair(key, value));
    values[std::cref(order_result->first)] = order_result;
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
  std::list<std::pair<const Key, Value>> order;
  std::unordered_map<std::reference_wrapper<const Key>, typename std::list<std::pair<const Key, Value>>::iterator, Hash, KeyEqual> values;
};

}  // namespace cnt
}  // namespace starlark

#endif  // CONTAINERS_LINKED_HASH_MAP_HPP_

