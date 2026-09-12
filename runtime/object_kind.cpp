// Copyright 2026 Lucas Mirelmann

#include "runtime/object_kind.hpp"

#include "runtime/starlark_types.hpp"

namespace starlark {
namespace runtime {

std::string_view kind_to_type_name(object_kind kind) {
  switch (kind) {
    case object_kind::kNone:
      return starlark_types::none_t;
    case object_kind::kBool:
      return starlark_types::bool_t;
    case object_kind::kInt:
    case object_kind::kBigInt:
      return starlark_types::int_t;
    case object_kind::kFloat:
      return starlark_types::float_t;
    case object_kind::kString:
      return starlark_types::string_t;
    case object_kind::kStringElems:
      return "string.elems";
    case object_kind::kStringElemOrds:
      return "string.elem_ords";
    case object_kind::kStringCodepoints:
      return "string.codepoints";
    case object_kind::kStringCodepointOrds:
      return "string.codepoint_ords";
    case object_kind::kBytes:
      return starlark_types::bytes_t;
    case object_kind::kBytesElems:
      return "bytes.elems";
    case object_kind::kBytesElemOrds:
      return "bytes.elem_ords";
    case object_kind::kList:
      return starlark_types::list_t;
    case object_kind::kTuple:
      return starlark_types::tuple_t;
    case object_kind::kDict:
      return starlark_types::dict_t;
    case object_kind::kSet:
      return starlark_types::set_t;
    case object_kind::kRange:
      return starlark_types::range_t;
    case object_kind::kFunction:
    case object_kind::kTestingFunction:
      return starlark_types::function_t;
    case object_kind::kBuiltinFunction:
      return starlark_types::builtin_function_or_method_t;
    case object_kind::kUnknown:
      return "";
  }
  return "";
}

bool same_starlark_type(object_kind lhs, object_kind rhs) {
  if (lhs == rhs) {
    return true;
  }
  if (is_int_kind(lhs) && is_int_kind(rhs)) {
    return true;
  }
  if ((lhs == object_kind::kFunction || lhs == object_kind::kTestingFunction) &&
      (rhs == object_kind::kFunction || rhs == object_kind::kTestingFunction)) {
    return true;
  }
  return false;
}

}  // namespace runtime
}  // namespace starlark
