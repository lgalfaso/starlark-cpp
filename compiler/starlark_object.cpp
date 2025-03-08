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

size_t starlark_hash::operator()(const starlark_obj* value) const {
  return value->hash();
}

bool starlark_equals_to::operator()(const starlark_obj* lhs, const starlark_obj* rhs) const {
  return lhs->equals(*rhs);
}

}  // namespace compiler
}  // namespace starlark


