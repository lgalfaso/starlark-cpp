// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_object.hpp"

#include <string>
#include <utility>

namespace starlark {
namespace compiler {

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
    if (top.action == printer_action::print_top && !stack.insert(top.obj).second) {
      top.obj->inner_repr(*this, printer_action::print_recursion);
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
    .action = printer_action::print_top,
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

}  // namespace compiler
}  // namespace starlark


