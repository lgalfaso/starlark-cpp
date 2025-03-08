// Copyright 2025 Lucas Mirelmann

#ifndef COMPILER_STARLARK_OBJECT_HPP_
#define COMPILER_STARLARK_OBJECT_HPP_

#include <unordered_set>
#include <string>
#include <string_view>
#include <vector>

#pragma GCC visibility push(default)

namespace starlark {
namespace compiler {

class starlark_obj;

enum class printer_action {
  print_top,
  print_element_separator,
  print_in_element_separator,
  print_final,
  print_single_element_final,
  print_recursion,
};

class printer {
 public:
  struct pending_task {
    const starlark_obj* obj = nullptr;
    printer_action action;
  };

  void append(std::string_view str);
  const std::string& value() const;
  void add_task(pending_task&& task);
  void run();

 private:
  std::string partial_value;
  std::vector<pending_task> tasks;
  std::unordered_set<const starlark_obj*> stack;
};

class starlark_obj {
 public:
  virtual ~starlark_obj();
  virtual std::string_view type() const = 0;
  virtual std::string str() const;
  std::string repr() const;
  virtual bool truthy() const = 0;
  virtual bool equals(const starlark_obj& other) const = 0;
  virtual int64_t hash() const = 0;

 protected:
  virtual bool inner_repr(printer& print, printer_action action) const = 0;

  friend class printer;
};

struct starlark_hash {
  size_t operator()(const starlark_obj* value) const;
};

struct starlark_equals_to {
  bool operator()(const starlark_obj* lhs, const starlark_obj* rhs) const;
};

}  // namespace compiler
}  // namespace starlark

#pragma GCC visibility pop

#endif  // COMPILER_STARLARK_OBJECT_HPP_

