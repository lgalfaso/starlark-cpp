// Copyright 2024-2025 Lucas Mirelmann

#ifndef CONTAINERS_LINKED_HASH_SET_HPP_
#define CONTAINERS_LINKED_HASH_SET_HPP_

#include <functional>
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
  typedef std::list<Key>::iterator iterator;
  typedef std::list<Key>::const_iterator const_iterator;
  typedef std::list<Key>::reverse_iterator reverse_iterator;
  typedef std::list<Key>::const_reverse_iterator const_reverse_iterator;

  linked_hash_set() {}

  linked_hash_set(const linked_hash_set& other) {
    for (const auto& value : other) {
      insert(value);
    }
  }

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

  std::pair<typename std::list<Key>::const_iterator, bool> insert(const Key& value) {
    auto result = values.find(value);
    if (result != values.end()) {
      // If the value already exists, then keep the previous value. This is consistent with
      // how Python handles sets.
      return std::make_pair(result->second, false);
    }
    auto order_result = order.insert(order.end(), value);
    values[std::cref(*order_result)] = order_result;
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
  std::list<Key> order;
  std::unordered_map<std::reference_wrapper<const Key>, typename std::list<Key>::iterator, Hash, KeyEqual> values;
};

}  // namespace cnt
}  // namespace starlark

#endif  // CONTAINERS_LINKED_HASH_SET_HPP_

