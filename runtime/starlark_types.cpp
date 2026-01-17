// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_types.hpp"

namespace starlark {
namespace runtime {

const char starlark_types::none_t[] = "NoneType";
const char starlark_types::bool_t[] = "bool";
const char starlark_types::int_t[] = "int";
const char starlark_types::float_t[] = "float";
const char starlark_types::string_t[] = "string";
const char starlark_types::bytes_t[] = "bytes";
const char starlark_types::list_t[] = "list";
const char starlark_types::tuple_t[] = "tuple";
const char starlark_types::dict_t[] = "dict";
const char starlark_types::set_t[] = "set";
const char starlark_types::function_t[] = "function";
const char starlark_types::builtin_function_or_method_t[] = "builtin_function_or_method";
const char starlark_types::range_t[] = "range";
const char starlark_types::int64[] = "int64";

}  // namespace runtime
}  // namespace starlark

