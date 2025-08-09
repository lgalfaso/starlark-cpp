// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_object.hpp"

#include <string>
#include <utility>

namespace starlark {
namespace runtime {

void printer::append(std::string_view str) {
  partial_value += str;
}

const std::string& printer::value() const {
  return partial_value;
}

void printer::add_task(pending_task&& task) {
  tasks.emplace_back(std::move(task));
}

void printer::run() {
  while (!tasks.empty()) {
    auto top = tasks.back();
    tasks.pop_back();
    if (top.action == printer_action::kPrintTop && !stack.insert(top.obj).second) {
      top.obj->inner_repr(*this, printer_action::kPrintRecursion);
      continue;
    }
    if (!top.obj->inner_repr(*this, top.action)) {
      stack.erase(top.obj);
    }
  }
}

void comparator::add_task(pending_task&& task) {
  tasks.emplace_back(std::move(task));
}

bool comparator::run() {
  while (!tasks.empty()) {
    auto top = tasks.back();
    tasks.pop_back();
    if (top.lhs == top.rhs) {
      continue;
    }
    if (executed_tasks.insert(top).second) {
      if (!top.lhs->inner_equals(*this, top.rhs)) {
        return false;
      }
    }
  }
  return true;
}

size_t comparator::pending_task_hash::operator()(const pending_task task) const {
  return hash_fn(task.lhs) ^ hash_fn(task.rhs);
}

bool comparator::pending_task_equals_to::operator()(const pending_task& lhs, const pending_task& rhs) const {
  return (lhs.lhs == rhs.lhs && lhs.rhs == rhs.rhs) ||
         (lhs.lhs == rhs.rhs && lhs.rhs == rhs.lhs);
}

starlark_obj::~starlark_obj() {}

std::string starlark_obj::str() const {
  return repr();
}

std::string starlark_obj::repr() const {
  printer print;
  print.add_task(printer::pending_task{
    .obj = this,
    .action = printer_action::kPrintTop,
  });
  print.run();
  return print.value();
}

bool starlark_obj::equals(const starlark_obj& other) const {
  comparator cmp;
  cmp.add_task(comparator::pending_task{
    .lhs = this,
    .rhs = &other,
  });
  return cmp.run();
}

int64_t starlark_obj::hash() const {
  auto candidate = inner_hash();
  if (std::holds_alternative<int64_t>(candidate)) {
    return std::get<int64_t>(candidate);
  }

  // This implementation is recursion-free.
  std::vector<std::vector<const starlark_obj*>> pending;
  std::vector<std::vector<int64_t>> done;
  pending_hash pending_hash_candidate = std::get<pending_hash>(candidate);
  pending.emplace_back(pending_hash_candidate.rbegin(), pending_hash_candidate.rend());
  done.emplace_back();
  while (true) {
    if (pending.back().empty()) {
      pending.pop_back();
      int64_t new_hash = starlark_hash(std::span<int64_t>(done.back().begin(), done.back().end()));
      done.pop_back();
      if (done.empty()) {
        return new_hash;
      }
      done.back().emplace_back(new_hash);
    } else {
      const starlark_obj* element = pending.back().back();
      pending.back().pop_back();
      candidate = element->inner_hash();
      if (std::holds_alternative<int64_t>(candidate)) {
        int64_t new_hash = std::get<int64_t>(candidate);
        if (new_hash == -1) {
          return new_hash;
        }
        done.back().emplace_back(new_hash);
      } else {
        pending_hash pending_hash_candidate = std::get<pending_hash>(candidate);
        pending.emplace_back(pending_hash_candidate.rbegin(), pending_hash_candidate.rend());
        done.emplace_back();
      }
    }
  }
}

size_t starlark_hash_op::operator()(const starlark_obj* value) const {
  return value->hash();
}

bool starlark_equals_to::operator()(const starlark_obj* lhs, const starlark_obj* rhs) const {
  return lhs->equals(*rhs);
}

int64_t starlark_hash(std::span<int64_t> values) {
  constexpr uint64_t hash_prime1 = 11400714785074694791UL;
  constexpr uint64_t hash_prime2 = 14029467366897019727UL;
  constexpr uint64_t hash_prime5 = 2870177450012600261UL;

  int64_t acc = hash_prime5;
  for (const auto& element_hash : values) {
    if (element_hash == -1) {
      return -1;
    }
    acc += element_hash * hash_prime2;
    acc = std::rotl<uint64_t>(acc, 31);
    acc *= hash_prime1;
  }
  acc += values.size() ^ (hash_prime5 ^ 3527539UL);
  if (acc == -1) {
    return 1546275796;
  }
  return acc;
}

}  // namespace runtime
}  // namespace starlark


