// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_TYPES_HPP_
#define RUNTIME_STARLARK_TYPES_HPP_

namespace starlark {
namespace runtime {

struct starlark_types {
  static const char none_t[];
  static const char bool_t[];
  static const char int_t[];
  static const char float_t[];
  static const char string_t[];
  static const char bytes_t[];
  static const char list_t[];
  static const char tuple_t[];
  static const char dict_t[];
  static const char set_t[];
  static const char function_t[];
  static const char builtin_function_or_method_t[];
  static const char range_t[];
};

}  // namespace runtime
}  // namespace starlark

#endif  // RUNTIME_STARLARK_TYPES_HPP_

