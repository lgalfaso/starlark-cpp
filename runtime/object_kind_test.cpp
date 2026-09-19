// Copyright 2026 Lucas Mirelmann

#include <gtest/gtest.h>

#include "runtime/object_kind.hpp"
#include "runtime/starlark_types.hpp"

using ::starlark::runtime::is_bytes_kind;
using ::starlark::runtime::is_int_kind;
using ::starlark::runtime::is_numeric_kind;
using ::starlark::runtime::is_string_kind;
using ::starlark::runtime::kind_to_type_name;
using ::starlark::runtime::object_kind;
using ::starlark::runtime::same_starlark_type;

namespace {

// Mirrors the old `obj->type() == starlark_types::string_t` check.
bool legacy_is_string_type(std::string_view type_name) {
  return type_name == starlark::runtime::starlark_types::string_t;
}

// Mirrors the old `obj->type() == starlark_types::bytes_t` check.
bool legacy_is_bytes_type(std::string_view type_name) {
  return type_name == starlark::runtime::starlark_types::bytes_t;
}

// Mirrors the old `lhs->type() == rhs->type()` check.
bool legacy_same_type(object_kind lhs, object_kind rhs) {
  return kind_to_type_name(lhs) == kind_to_type_name(rhs);
}

TEST(ObjectKind, StringPredicateMatchesLegacyStringTypeCheck) {
  EXPECT_TRUE(is_string_kind(object_kind::kString));
  EXPECT_TRUE(legacy_is_string_type(kind_to_type_name(object_kind::kString)));

  const object_kind string_views[] = {
      object_kind::kStringElems,
      object_kind::kStringElemOrds,
      object_kind::kStringCodepoints,
      object_kind::kStringCodepointOrds,
  };
  for (object_kind kind : string_views) {
    EXPECT_FALSE(is_string_kind(kind));
    EXPECT_FALSE(legacy_is_string_type(kind_to_type_name(kind)));
    EXPECT_NE(kind_to_type_name(kind), starlark::runtime::starlark_types::string_t);
  }
}

TEST(ObjectKind, BytesPredicateMatchesLegacyBytesTypeCheck) {
  EXPECT_TRUE(is_bytes_kind(object_kind::kBytes));
  EXPECT_TRUE(legacy_is_bytes_type(kind_to_type_name(object_kind::kBytes)));

  const object_kind bytes_views[] = {
      object_kind::kBytesElems,
      object_kind::kBytesElemOrds,
  };
  for (object_kind kind : bytes_views) {
    EXPECT_FALSE(is_bytes_kind(kind));
    EXPECT_FALSE(legacy_is_bytes_type(kind_to_type_name(kind)));
    EXPECT_NE(kind_to_type_name(kind), starlark::runtime::starlark_types::bytes_t);
  }
}

TEST(ObjectKind, IntPredicateGroupsIntAndBigInt) {
  EXPECT_TRUE(is_int_kind(object_kind::kInt));
  EXPECT_TRUE(is_int_kind(object_kind::kBigInt));
  EXPECT_EQ(kind_to_type_name(object_kind::kInt), kind_to_type_name(object_kind::kBigInt));
}

TEST(ObjectKind, SameStarlarkTypeMatchesLegacyTypeEquality) {
  const object_kind kinds[] = {
      object_kind::kNone,
      object_kind::kBool,
      object_kind::kInt,
      object_kind::kBigInt,
      object_kind::kFloat,
      object_kind::kString,
      object_kind::kStringElems,
      object_kind::kStringElemOrds,
      object_kind::kStringCodepoints,
      object_kind::kStringCodepointOrds,
      object_kind::kBytes,
      object_kind::kBytesElems,
      object_kind::kBytesElemOrds,
      object_kind::kList,
      object_kind::kTuple,
      object_kind::kDict,
      object_kind::kSet,
      object_kind::kRange,
      object_kind::kBuiltinFunction,
      object_kind::kFunction,
      object_kind::kTestingFunction,
  };

  for (object_kind lhs : kinds) {
    for (object_kind rhs : kinds) {
      EXPECT_EQ(same_starlark_type(lhs, rhs), legacy_same_type(lhs, rhs))
          << "lhs=" << static_cast<int>(lhs) << " rhs=" << static_cast<int>(rhs);
    }
  }
}

TEST(ObjectKind, NumericKindPredicate) {
  EXPECT_TRUE(is_numeric_kind(object_kind::kInt));
  EXPECT_TRUE(is_numeric_kind(object_kind::kBigInt));
  EXPECT_TRUE(is_numeric_kind(object_kind::kFloat));
  EXPECT_FALSE(is_numeric_kind(object_kind::kString));
}

TEST(ObjectKind, ElemViewsKeepDistinctTypeNames) {
  EXPECT_EQ(kind_to_type_name(object_kind::kStringElems), "string.elems");
  EXPECT_EQ(kind_to_type_name(object_kind::kStringElemOrds), "string.elem_ords");
  EXPECT_EQ(kind_to_type_name(object_kind::kStringCodepoints), "string.codepoints");
  EXPECT_EQ(kind_to_type_name(object_kind::kStringCodepointOrds), "string.codepoint_ords");
  EXPECT_EQ(kind_to_type_name(object_kind::kBytesElems), "bytes.elems");
  EXPECT_EQ(kind_to_type_name(object_kind::kBytesElemOrds), "bytes.elem_ords");
}

}  // namespace
