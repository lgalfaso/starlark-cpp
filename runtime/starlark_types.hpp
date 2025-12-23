// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_TYPES_HPP_
#define RUNTIME_STARLARK_TYPES_HPP_

#include <string>

namespace starlark {
namespace runtime {

struct starlark_types {
  const static std::string none_t;
  const static std::string bool_t;
  const static std::string int_t;
  const static std::string float_t;
  const static std::string string_t;
  const static std::string bytes_t;
  const static std::string list_t;
  const static std::string tuple_t;
  const static std::string dict_t;
  const static std::string set_t;
  const static std::string function_t;
  const static std::string builtin_function_or_method_t;
  const static std::string range_t;
};

}  // namespace runtime
}  // namespace starlark

#endif  // RUNTIME_STARLARK_TYPES_HPP_

