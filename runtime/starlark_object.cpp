// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_object.hpp"

#include <bit>
#include <map>
#include <string>
#include <utility>
#include <vector>

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

void equals_comparator::add_task(pending_task&& task) {
  tasks.emplace_back(std::move(task));
}

bool equals_comparator::run() {
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

size_t equals_comparator::pending_task_hash::operator()(const pending_task task) const {
  return hash_fn(task.lhs) ^ hash_fn(task.rhs);
}

bool equals_comparator::pending_task_equals_to::operator()(const pending_task& lhs, const pending_task& rhs) const {
  return (lhs.lhs == rhs.lhs && lhs.rhs == rhs.rhs) ||
         (lhs.lhs == rhs.rhs && lhs.rhs == rhs.lhs);
}

void order_comparator::add_task(pending_task&& task) {
  tasks.emplace_back(std::move(task));
}

int order_comparator::run(std::string_view op, error_fn* error_callback) {
  while (!tasks.empty()) {
    auto top = tasks.back();
    tasks.pop_back();
    switch (top.type) {
      case pending_task_type::kEvaluate:
        top.lhs->inner_cmp(*this, top.rhs, op, error_callback);
        break;
      case pending_task_type::kLessThan:
        return -1;
      case pending_task_type::kGreaterThan:
        return 1;
      case pending_task_type::kFail:
        return 0;
    }
  }
  return 0;
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
  equals_comparator cmp;
  cmp.add_task(equals_comparator::pending_task{
    .lhs = this,
    .rhs = &other,
  });
  return cmp.run();
}

int starlark_obj::cmp(const starlark_obj& other, std::string_view op, error_fn* error_callback) const {
  order_comparator cmp;
  cmp.add_task(order_comparator::pending_task{
    .type = order_comparator::pending_task_type::kEvaluate,
    .lhs = this,
    .rhs = &other,
  });
  return cmp.run(op, error_callback);
}

int64_t starlark_obj::hash() const {
  // Replace this with -1 to disallow the hashing of objects that contain themself.
  const int64_t recursion_replacement = 1609587929392839161L;

  auto candidate = inner_hash();
  if (std::holds_alternative<int64_t>(candidate)) {
    return std::get<int64_t>(candidate);
  }

  // This implementation is recursion-free.
  std::vector<std::vector<const starlark_obj*>> pending;
  std::vector<std::vector<int64_t>> done;
  std::map<const starlark_obj*, int64_t> cache;
  std::vector<const starlark_obj*> current;

  pending_hash pending_hash_candidate = std::get<pending_hash>(candidate);
  pending.emplace_back(pending_hash_candidate.rbegin(), pending_hash_candidate.rend());
  done.emplace_back();
  current.push_back(this);
  while (true) {
    if (pending.back().empty()) {
      pending.pop_back();
      int64_t new_hash = starlark_hash(std::span<int64_t>(done.back().begin(), done.back().end()));
      done.pop_back();
      cache[current.back()] = new_hash;
      current.pop_back();
      if (done.empty()) {
        return new_hash;
      }
      done.back().emplace_back(new_hash);
    } else {
      const starlark_obj* element = pending.back().back();
      pending.back().pop_back();
      if (cache.contains(element)) {
        int64_t new_hash = cache[element];
        if (new_hash == -1) {
          return new_hash;
        }
        done.back().emplace_back(new_hash);
      } else {
        // This is added to detect hash recursions.
        cache[element] = recursion_replacement;
        candidate = element->inner_hash();
        if (std::holds_alternative<int64_t>(candidate)) {
          int64_t new_hash = std::get<int64_t>(candidate);
          if (new_hash == -1) {
            return new_hash;
          }
          done.back().emplace_back(new_hash);
          cache[element] = new_hash;
        } else {
          pending_hash pending_hash_candidate = std::get<pending_hash>(candidate);
          pending.emplace_back(pending_hash_candidate.rbegin(), pending_hash_candidate.rend());
          done.emplace_back();
          current.push_back(element);
        }
      }
    }
  }
}

void starlark_obj::freeze() {
  if (freezed) {
    return;
  }
  freezed = true;
  std::vector<starlark_obj*> to_freeze;
  inner_freeze(to_freeze);
  while (!to_freeze.empty()) {
    auto* element = to_freeze.back();
    to_freeze.pop_back();
    if (element->freezed) {
      continue;
    }
    element->freezed = true;
    element->inner_freeze(to_freeze);
  }
}

starlark_obj* starlark_obj::call(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args, error_fn* error_callback) {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: '{}' object is not callable", type()));
  }
  return nullptr;
}

void starlark_obj::unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn* error_callback) {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: cannot unpack non-iterable {} object", type()));
  }
}

starlark_obj* starlark_obj::unary_plus(google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: bad operand type for unary +: '{}'", type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::unary_minus(google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: bad operand type for unary -: '{}'", type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::unary_tilde(google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: bad operand type for unary ~: '{}'", type()));
  }
  return nullptr;
}

bool starlark_obj::binary_in(const starlark_obj& other, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: argument of type '{}' is not a container or iterable", type()));
  }
  return false;
}

starlark_obj* starlark_obj::binary_lshift(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for <<: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_rshift(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for >>: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_and(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for &: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_pipe(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for |: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_hat(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for ^: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_plus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for +: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_minus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for -: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_star(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for *: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for /: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_slash_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for //: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

starlark_obj* starlark_obj::binary_percent(const starlark_obj& other, google::protobuf::Arena& arena, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: unsupported operand type(s) for %: '{}' and '{}'", type(), other.type()));
  }
  return nullptr;
}

void starlark_obj::inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn* error_callback) const {
  if (error_callback != nullptr) {
    error_callback->add_error(std::format("TypeError: '{}' not supported between instances of '{}' and '{}'", op, type(), other->type()));
  }
  comp.add_task(order_comparator::pending_task{
    .type = order_comparator::pending_task_type::kFail,
  });
}

void starlark_obj::inner_freeze(std::vector<starlark_obj*>& to_freeze) {
  return;
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


