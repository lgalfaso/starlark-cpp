// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_OBJECT_HPP_
#define RUNTIME_STARLARK_OBJECT_HPP_

#include <map>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

class starlark_obj;

enum class printer_action {
  kPrintTop,
  kPrintElementSeparator,
  kPrintInElementSeparator,
  kPrintFinal,
  kPrintRecursion,
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

class comparator {
 public:
  struct pending_task {
    const starlark_obj* lhs = nullptr;
    const starlark_obj* rhs = nullptr;
    bool operator==(const pending_task& other) const = default;
  };

  void add_task(pending_task&& task);
  bool run();

 private:
  struct pending_task_hash {
    size_t operator()(const pending_task task) const;

    std::hash<const starlark_obj*> hash_fn;
  };

  struct pending_task_equals_to {
    bool operator()(const pending_task& lhs, const pending_task& rhs) const;
  };

  std::vector<pending_task> tasks;
  std::unordered_set<pending_task, pending_task_hash, pending_task_equals_to> executed_tasks;
};

class starlark_obj {
 public:
  virtual ~starlark_obj();
  virtual std::string_view type() const = 0;
  virtual std::string str() const;
  std::string repr() const;
  virtual bool truthy() const = 0;
  bool equals(const starlark_obj& other) const;
  int64_t hash() const;
  virtual starlark_obj* call(const std::vector<starlark_obj*>& pos_args, const std::map<std::string, starlark_obj*>& named_args);

 protected:
  typedef std::span<const starlark_obj* const> pending_hash;

  virtual bool inner_repr(printer& print, printer_action action) const = 0;
  virtual bool inner_equals(comparator& comp, const starlark_obj* other) const = 0;
  virtual std::variant<int64_t, pending_hash> inner_hash() const = 0;

  friend class printer;
  friend class comparator;
};

struct starlark_hash_op {
  size_t operator()(const starlark_obj* value) const;
};

struct starlark_equals_to {
  bool operator()(const starlark_obj* lhs, const starlark_obj* rhs) const;
};

int64_t starlark_hash(std::span<int64_t> values);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_OBJECT_HPP_

