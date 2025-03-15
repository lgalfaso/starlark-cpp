// Copyright 2025 Lucas Mirelmann

#include "compiler/starlark_tuple.hpp"

#include <bit>
#include <string>

namespace starlark {
namespace compiler {

std::string_view starlark_tuple::type() const {
  return "tuple";
}

bool starlark_tuple::inner_repr(printer& print, printer_action action) const {
  switch (action) {
    case printer_action::print_top: {
      if (values.size() == 0) {
        print.append("()");
        return false;
      }
      print.append("(");
      printer_action new_action = printer_action::print_final;
      for (auto it = values.rbegin(); it != values.rend(); ++it) {
        print.add_task(printer::pending_task{
          .obj = this,
          .action = new_action,
        });
        print.add_task(printer::pending_task{
          .obj = *it,
          .action = printer_action::print_top,
        });
        new_action = printer_action::print_element_separator;
      }
      return true;
    }
    case printer_action::print_element_separator:
    case printer_action::print_in_element_separator:
      print.append(", ");
      return true;
    case printer_action::print_final:
      if (values.size() == 1) {
        print.append(",)");
      } else {
        print.append(")");
      }
      return false;
    case printer_action::print_recursion:
      print.append("(...)");
      return false;
  }
}

bool starlark_tuple::inner_equals(comparator& comp, const starlark_obj* other) const {
  if (type() != other->type()) {
    return false;
  }
  const starlark_tuple* n_other = reinterpret_cast<const starlark_tuple*>(other);
  if (values.size() != n_other->values.size()) {
    return false;
  }
  for (int i = 0; i < values.size(); ++i) {
    comp.add_task(comparator::pending_task{
      .lhs = values[i],
      .rhs = n_other->values[i],
    });
  }
  return true;
}

bool starlark_tuple::truthy() const {
  return !values.empty();
}

int64_t starlark_tuple::hash() const {
  constexpr uint64_t hash_prime1 = 11400714785074694791UL;
  constexpr uint64_t hash_prime2 = 14029467366897019727UL;
  constexpr uint64_t hash_prime5 = 2870177450012600261UL;

  // TODO(lmirelmann): Implement without recursion.
  int64_t acc = hash_prime5;
  for (const auto& element : values) {
    int64_t element_hash = element->hash();
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

starlark_tuple& starlark_tuple::add(starlark_obj* element) {
  values.push_back(element);
  return *this;
}

}  // namespace compiler
}  // namespace starlark


