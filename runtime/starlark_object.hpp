// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_OBJECT_HPP_
#define RUNTIME_STARLARK_OBJECT_HPP_

#include <functional>
#include <map>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <variant>
#include <vector>

#include "bigint/number.hpp"
#include "containers/linked_hash_map.hpp"
#include "google/protobuf/arena.h"
#include "runtime/error_fn.hpp"

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

class equals_comparator {
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

class order_comparator {
 public:
  enum class pending_task_type {
    kEvaluate,
    kLessThan,
    kGreaterThan,
    kFail,
  };
  void add_task(pending_task_type task_type);
  void add_task(const starlark_obj* lhs, const starlark_obj* rhs);
  int run(std::string_view op, error_fn& error_callback);

 private:
  struct pending_task {
    const pending_task_type type;
    const starlark_obj* lhs = nullptr;
    const starlark_obj* rhs = nullptr;
  };

  std::vector<pending_task> tasks;
};

class starlark_iterator {
 public:
  virtual ~starlark_iterator();
  virtual bool has_next() const = 0;
  virtual starlark_obj* next() = 0;
  virtual void end_iterator() = 0;
};

enum class starlark_numeric_type {
  kInt64,
  kBigInt,
  kFloat,
  kNotNumeric
};

struct starlark_hash_op {
  size_t operator()(const starlark_obj* value) const;
};

struct starlark_equals_to {
  bool operator()(const starlark_obj* lhs, const starlark_obj* rhs) const;
};

class starlark_obj {
 public:
  typedef std::vector<starlark_obj*> pos_args_t;
  typedef starlark::cnt::linked_hash_map<std::string_view, starlark_obj*, std::hash<std::string_view>, std::equal_to<std::string_view>> named_args_t;
  typedef starlark_obj* (fn)(starlark_obj* this_obj, const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);

  starlark_obj();
  virtual ~starlark_obj();
  virtual std::string_view type() const = 0;
  virtual std::string str() const;
  std::string repr() const;
  virtual bool truthy() const = 0;
  virtual bool primitive() const;
  virtual const std::vector<std::string>& dir() const;
  virtual const std::map<std::string, fn*, std::less<>>& methods_meta() const;
  bool equals(const starlark_obj& other) const;
  int cmp(const starlark_obj& other, std::string_view op, error_fn& error_callback) const;
  int64_t hash() const;
  void freeze();
  virtual starlark_obj* call(const pos_args_t& pos_args, const named_args_t& named_args, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void unpack(int32_t number_of_elements, std::vector<starlark_obj*>& consumer, error_fn& error_callback);

  virtual starlark_obj* unary_plus(google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* unary_minus(google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* unary_tilde(google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual bool binary_in(const starlark_obj& other, error_fn& error_callback) const;
  virtual starlark_obj* plus_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* minus_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* star_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* slash_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* slash_slash_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* percent_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* ampersand_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* pipe_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* hat_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* less_less_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* greater_greater_equals_assign(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* binary_plus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_minus(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_star(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_slash_slash(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_percent(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_and(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_pipe(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_hat(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_lshift(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual starlark_obj* binary_rshift(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;

  virtual int64_t len(bool produce_error, error_fn& error_callback) const;
  virtual starlark_iterator* get_iterator(bool produce_error, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual starlark_obj* index(const starlark_obj& other, google::protobuf::Arena& arena, error_fn& error_callback) const;
  virtual void index_assign(const starlark_obj& idx, starlark_obj& element, error_fn& error_callback);
  virtual starlark_obj* dot(std::string_view field_name, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void dot_assign(std::string_view field_name, starlark_obj& element, error_fn& error_callback);

  virtual void slice_range_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_plus_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_minus_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_star_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_slash_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_slash_slash_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_percent_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_ampersand_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_pipe_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_hat_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_less_less_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);
  virtual void slice_range_greater_greater_equals_assign(const starlark_obj& start, const starlark_obj& stop, const starlark_obj& stride, const starlark_obj& element, google::protobuf::Arena& arena, error_fn& error_callback);

  virtual starlark_numeric_type numeric_type() const;
  virtual int64_t as_int64() const;
  virtual const starlark::bigint::number& as_bigint() const;
  virtual double as_float() const;
  virtual std::string_view as_string() const;
  virtual starlark_obj* get_attr(bool produce_error, std::string_view attribute, google::protobuf::Arena& arena, error_fn& error_callback);

 protected:
  typedef std::span<const starlark_obj* const> pending_hash;
  bool freezed;

  virtual bool inner_repr(printer& print, printer_action action) const = 0;
  virtual bool inner_equals(equals_comparator& comp, const starlark_obj* other) const = 0;
  virtual void inner_cmp(order_comparator& comp, const starlark_obj* other, std::string_view op, error_fn& error_callback) const;
  virtual std::variant<int64_t, pending_hash> inner_hash() const = 0;
  virtual void inner_freeze(std::vector<starlark_obj*>& to_freeze);
  int64_t inner_index(const starlark_obj& other, int64_t obj_len, error_fn& error_callback) const;

 private:
  static const std::map<std::string, fn*, std::less<>>& method_refs();
  static const std::vector<std::string>& attributes();

  friend class printer;
  friend class equals_comparator;
  friend class order_comparator;
};

int64_t starlark_hash(std::span<int64_t> values);
starlark_obj* create_function(google::protobuf::Arena &arena, starlark_obj* this_obj, starlark_obj::fn native_fn, std::string_view fn_name);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_OBJECT_HPP_

