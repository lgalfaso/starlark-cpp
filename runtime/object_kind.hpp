// Copyright 2026 Lucas Mirelmann

#ifndef RUNTIME_OBJECT_KIND_HPP_
#define RUNTIME_OBJECT_KIND_HPP_

#include <cstdint>

#include <string_view>

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

enum class object_kind : uint8_t {
  kUnknown = 0,
  kNone = 1,
  kBool = 2,
  kInt = 3,
  kBigInt = 4,
  kFloat = 5,
  kString = 6,
  kStringElems = 7,
  kStringElemOrds = 8,
  kStringCodepoints = 9,
  kStringCodepointOrds = 10,
  kBytes = 11,
  kBytesElems = 12,
  kBytesElemOrds = 13,
  kList = 14,
  kTuple = 15,
  kDict = 16,
  kSet = 17,
  kRange = 18,
  kBuiltinFunction = 19,
  kFunction = 20,
  kTestingFunction = 21,
};

std::string_view kind_to_type_name(object_kind kind);
bool same_starlark_type(object_kind lhs, object_kind rhs);

inline bool is_none_kind(object_kind kind) { return kind == object_kind::kNone; }
inline bool is_bool_kind(object_kind kind) { return kind == object_kind::kBool; }
inline bool is_int_kind(object_kind kind) { return kind == object_kind::kInt || kind == object_kind::kBigInt; }
inline bool is_float_kind(object_kind kind) { return kind == object_kind::kFloat; }
inline bool is_string_kind(object_kind kind) { return kind == object_kind::kString; }
inline bool is_bytes_kind(object_kind kind) { return kind == object_kind::kBytes; }
inline bool is_list_kind(object_kind kind) { return kind == object_kind::kList; }
inline bool is_tuple_kind(object_kind kind) { return kind == object_kind::kTuple; }
inline bool is_dict_kind(object_kind kind) { return kind == object_kind::kDict; }
inline bool is_set_kind(object_kind kind) { return kind == object_kind::kSet; }
inline bool is_range_kind(object_kind kind) { return kind == object_kind::kRange; }
inline bool is_function_kind(object_kind kind) { return kind == object_kind::kFunction; }
inline bool is_builtin_function_kind(object_kind kind) { return kind == object_kind::kBuiltinFunction; }
inline bool is_numeric_kind(object_kind kind) {
  return kind == object_kind::kInt || kind == object_kind::kBigInt || kind == object_kind::kFloat;
}

// Types that cannot contain other starlark objects and never need cycle tracking for ==.
inline bool is_leaf_for_equals(object_kind kind) {
  switch (kind) {
    case object_kind::kNone:
    case object_kind::kBool:
    case object_kind::kInt:
    case object_kind::kBigInt:
    case object_kind::kFloat:
    case object_kind::kString:
    case object_kind::kStringElems:
    case object_kind::kStringElemOrds:
    case object_kind::kStringCodepoints:
    case object_kind::kStringCodepointOrds:
    case object_kind::kBytes:
    case object_kind::kBytesElems:
    case object_kind::kBytesElemOrds:
    case object_kind::kRange:
      return true;
    default:
      return false;
  }
}

// Ordinal values preserved for the JIT runtime ABI (formerly starlark_numeric_type).
constexpr int32_t kNumericAbiInt64 = 0;
constexpr int32_t kNumericAbiBigInt = 1;
constexpr int32_t kNumericAbiFloat = 2;
constexpr int32_t kNumericAbiNotNumeric = 3;

inline int32_t kind_to_numeric_abi(object_kind kind) {
  switch (kind) {
    case object_kind::kInt:
      return kNumericAbiInt64;
    case object_kind::kBigInt:
      return kNumericAbiBigInt;
    case object_kind::kFloat:
      return kNumericAbiFloat;
    default:
      return kNumericAbiNotNumeric;
  }
}

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_OBJECT_KIND_HPP_
