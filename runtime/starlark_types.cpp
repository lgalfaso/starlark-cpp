// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_types.hpp"

namespace starlark {
namespace runtime {

const std::string starlark_types::none_t = "NoneType";
const std::string starlark_types::bool_t = "bool";
const std::string starlark_types::int_t = "int";
const std::string starlark_types::float_t = "float";
const std::string starlark_types::string_t = "string";
const std::string starlark_types::bytes_t = "bytes";
const std::string starlark_types::list_t = "list";
const std::string starlark_types::tuple_t = "tuple";
const std::string starlark_types::dict_t = "dict";
const std::string starlark_types::set_t = "set";
const std::string starlark_types::function_t = "function";
const std::string starlark_types::range_t = "range";

}  // namespace runtime
}  // namespace starlark

