// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_function.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"
#include "runtime/starlark_types.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
using ::starlark::bigint::parse_number;
using ::starlark::runtime::context;
using ::starlark::runtime::starlark_bigint;
using ::starlark::runtime::starlark_bool;
using ::starlark::runtime::starlark_built_in_function;
using ::starlark::runtime::starlark_bytes;
using ::starlark::runtime::starlark_dictionary;
using ::starlark::runtime::starlark_float;
using ::starlark::runtime::starlark_function;
using ::starlark::runtime::starlark_integer;
using ::starlark::runtime::starlark_list;
using ::starlark::runtime::starlark_none;
using ::starlark::runtime::starlark_obj;
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_tuple;
using ::starlark::runtime::starlark_types;
using ::starlark::testing::error_handler;
using ::std::literals::string_view_literals::operator""sv;
using ::testing::Contains;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

TEST(StarlarkString, Type) {
  EXPECT_EQ("string", starlark_string(""sv).type());
}

TEST(StarlarkString, Primitve) {
  EXPECT_TRUE(starlark_string(""sv).primitive());
}

TEST(StarlarkString, Str) {
  EXPECT_EQ("abcdef", starlark_string("abcdef"sv).str());
  EXPECT_EQ("fedcba", starlark_string("fedcba"sv).str());
}

TEST(StarlarkString, Repr) {
  // TODO(lmirelmann): Would be nice to have a test that checks the encoding of all characters.
  EXPECT_EQ("\"abcdef\"", starlark_string("abcdef"sv).repr());
  EXPECT_EQ("\"'\"", starlark_string("'"sv).repr());
  EXPECT_EQ("\"'\\\"\"", starlark_string("'\""sv).repr());
  EXPECT_EQ("\"\\t\\r\\n\"", starlark_string("\t\r\n"sv).repr());
  EXPECT_EQ("\"\\x01\\x02\\x7f\"", starlark_string("\001\002\177"sv).repr());
  EXPECT_EQ("\"\\x90\"", starlark_string("\302\220"sv).repr());
  EXPECT_EQ("\"\xC3\xA0\"", starlark_string("\303\240"sv).repr());
  EXPECT_EQ("\"\xC8\xB4\"", starlark_string("\310\264"sv).repr());
  EXPECT_EQ("\"\\u0378\"", starlark_string("\315\270"sv).repr());
  EXPECT_EQ("\"\\ud800\"", starlark_string("\355\240\200"sv).repr());
  EXPECT_EQ("\"\\U000101c7\"", starlark_string("\360\220\207\207"sv).repr());
  EXPECT_EQ("\"\xf0\"", starlark_string(std::string("🙂"sv).substr(0, 1)).repr());
  EXPECT_EQ("\"\\ufeff\"", starlark_string("\xef\xbb\xbf"sv).repr());
}

TEST(StarlarkString, Truthy) {
  EXPECT_FALSE(starlark_string(""sv).truthy());
  EXPECT_TRUE(starlark_string("a"sv).truthy());
}

TEST(StarlarkString, Equals) {
  EXPECT_TRUE(starlark_string(""sv).equals(starlark_string(""sv)));
  EXPECT_TRUE(starlark_string("a"sv).equals(starlark_string("a"sv)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_string("a"sv)));
  // This is the NFKC decomposition.
  EXPECT_FALSE(starlark_string("\u03C9\u0301"sv).equals(starlark_string("\u03CE"sv)));

  EXPECT_FALSE(starlark_string(""sv).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_bool(false)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_bytes(""sv)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_function()));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_list(0)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_none()));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_range(0, 1, 1)));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_set()));
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_tuple(0)));

  EXPECT_FALSE(starlark_string("0"sv).equals(starlark_integer(0)));
}

TEST(StarlarkString, Hash) {
  EXPECT_EQ(0, starlark_string(""sv).hash());
  EXPECT_EQ(0x539375b79c9167a6, starlark_string(std::string("\000", 1)).hash());
  EXPECT_EQ(0x19ac88fb2429e222, starlark_string("a"sv).hash());
  EXPECT_EQ(0x12d48511de046bbc, starlark_string("ab"sv).hash());
  EXPECT_EQ(0x53d7cad63d3282ff, starlark_string("abc"sv).hash());
  EXPECT_EQ(0x92981e87e70229, starlark_string("abcd"sv).hash());
  EXPECT_EQ(0xabac8f555b5e912, starlark_string("abcde"sv).hash());
  EXPECT_EQ(0x63667dc70d4dbf89, starlark_string("abcdef"sv).hash());
  EXPECT_EQ(0x3539f9b81c64a336, starlark_string("abcdefg"sv).hash());
  EXPECT_EQ(-0x7f95eee8bc7b632d, starlark_string("abcdefgh"sv).hash());
  EXPECT_EQ(-0x642099e9fba65ad0, starlark_string("abcdefghi"sv).hash());
  EXPECT_EQ(-0x669943fc9f1993d5, starlark_string("abcdefghij"sv).hash());
  EXPECT_EQ(0x5e5235ef5071d1c8, starlark_string("abcdefghijk"sv).hash());
  EXPECT_EQ(-0x574fd3e31dd1b6c0, starlark_string("abcdefghijkl"sv).hash());
  EXPECT_EQ(0x348e1f9d6e3c67a9, starlark_string("abcdefghijklm"sv).hash());
  EXPECT_EQ(0x4b59cf75df0ea525, starlark_string("abcdefghijklmn"sv).hash());
  EXPECT_EQ(0x7c0d99416b677716, starlark_string("abcdefghijklmno"sv).hash());
  EXPECT_EQ(0x651dcfdc9b304273, starlark_string("abcdefghijklmnop"sv).hash());
  EXPECT_EQ(-0x706278fdd6f9101, starlark_string("abcdefghijklmnopq"sv).hash());
  EXPECT_EQ(0x310a2e3dd291e1a6, starlark_string("abcdefghijklmnopqr"sv).hash());
  EXPECT_EQ(0x76e0c0d29cc59331, starlark_string("abcdefghijklmnopqrs"sv).hash());
  EXPECT_EQ(-0x89ec229ad3ce95e, starlark_string("abcdefghijklmnopqrst"sv).hash());
  EXPECT_EQ(-0x3421666900cc950, starlark_string("abcdefghijklmnopqrstu"sv).hash());
  EXPECT_EQ(0x7be37d6337827631, starlark_string("abcdefghijklmnopqrstuv"sv).hash());
  EXPECT_EQ(-0xd93501f3830d3c5, starlark_string("abcdefghijklmnopqrstuvw"sv).hash());
  EXPECT_EQ(-0x1ec56e52deeb2ed9, starlark_string("abcdefghijklmnopqrstuvwx"sv).hash());
  EXPECT_EQ(0x2928ab0caeae7b0d, starlark_string("abcdefghijklmnopqrstuvwxy"sv).hash());
  EXPECT_EQ(0x732f1d3705b6dfa3, starlark_string("abcdefghijklmnopqrstuvwxyz"sv).hash());
  EXPECT_EQ(-0x3d305c344304b1, starlark_string("abcdefghijklmnopqrstuvwxyz0"sv).hash());
  EXPECT_EQ(-0x4bbc24288075d499, starlark_string("abcdefghijklmnopqrstuvwxyz01"sv).hash());
  EXPECT_EQ(0x630596a3267cd0f9, starlark_string("abcdefghijklmnopqrstuvwxyz012"sv).hash());
  EXPECT_EQ(-0x7176a79e04f34841, starlark_string("abcdefghijklmnopqrstuvwxyz0123"sv).hash());
  EXPECT_EQ(0x7b3b18b2511d145a, starlark_string("abcdefghijklmnopqrstuvwxyz01234"sv).hash());
  EXPECT_EQ(-0x116d8bb10196190f, starlark_string("abcdefghijklmnopqrstuvwxyz012345"sv).hash());
  EXPECT_EQ(-0x7a27473192c1723a, starlark_string("abcdefghijklmnopqrstuvwxyz0123456"sv).hash());
  EXPECT_EQ(0x731309efc4441b64, starlark_string("abcdefghijklmnopqrstuvwxyz01234567"sv).hash());
  EXPECT_EQ(-0x41044b627d75b155, starlark_string("abcdefghijklmnopqrstuvwxyz012345678"sv).hash());
  EXPECT_EQ(-0x38976cda9bea7d32, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789"sv).hash());
  EXPECT_EQ(0x6079104dbaf33098, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789@"sv).hash());
  EXPECT_EQ(-0x6f9048eb54a00388, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789@!"sv).hash());
}

TEST(StarlarkString, Order) {
  error_handler error_callback;

  EXPECT_THAT(starlark_string(""sv).cmp(starlark_string(""sv), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_string(""sv).cmp(starlark_string("a"sv), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_string("a"sv).cmp(starlark_string("a"sv), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_string("a"sv).cmp(starlark_string(""sv), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_string("a"sv).cmp(starlark_string("b"sv), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_string("b"sv).cmp(starlark_string("a"sv), "cmp", error_callback), Gt(0));
}

TEST(StarlarkString, OrderErrors) {
  error_handler error_callback;
  EXPECT_FALSE(starlark_string(""sv).cmp(starlark_bytes(""sv), "<", error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'string' and 'bytes'");
}

TEST(StarlarkString, BinaryIn) {
  error_handler error_callback;
  EXPECT_TRUE(starlark_string(""sv).binary_in(starlark_string(""sv), error_callback));
  EXPECT_TRUE(starlark_string("a"sv).binary_in(starlark_string(""sv), error_callback));
  EXPECT_FALSE(starlark_string("a"sv).binary_in(starlark_string("b"sv), error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkString, BinaryInErrors) {
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string("a"sv).binary_in(starlark_integer('b'), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string("a"sv).binary_in(starlark_integer('a'), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string("a"sv).binary_in(starlark_bigint('a'), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_string(""sv), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: a bytes-like object is required, not 'string'");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_integer(-1), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_integer(256), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_bigint(-1), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_bigint(256), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
}

TEST(StarlarkString, BinaryPlus) {
  starlark_string str_1("abc"sv);
  starlark_string str_2("def"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = str_1.binary_plus(str_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(str_1.str(), "abc");
  EXPECT_EQ(result->str(), "abcdef");
}

TEST(StarlarkString, BinaryPlusNotList) {
  starlark_string str("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = str.binary_plus(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't concat tuple to string");
}

TEST(StarlarkString, PlusEqualsAssign) {
  starlark_string str_1("abc"sv);
  starlark_string str_2("def"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = str_1.plus_equals_assign(str_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(str_1.str(), "abc");
  EXPECT_EQ(result->str(), "abcdef");
}

TEST(StarlarkString, PlusEqualsAssignNotList) {
  starlark_string str("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = str.plus_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't concat tuple to string");
}

TEST(StarlarkString, BinaryStar) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_string str0(""sv);
  starlark_string str("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = str.binary_star(two, ctx, error_callback);
  auto* result_2 = str.binary_star(three, ctx, error_callback);
  auto* result_3 = str.binary_star(minus_two, ctx, error_callback);
  auto* result_4 = str.binary_star(minus_one, ctx, error_callback);
  auto* result_5 = str0.binary_star(big, ctx, error_callback);
  auto* result_6 = str0.binary_star(two, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "abcabc");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "abcabcabc");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "");
  ASSERT_NE(result_6, nullptr);
  EXPECT_EQ(result_6->str(), "");
}

TEST(StarlarkString, BinaryStarReverse) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_string str0(""sv);
  starlark_string str("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = two.binary_star(str, ctx, error_callback);
  auto* result_2 = three.binary_star(str, ctx, error_callback);
  auto* result_3 = minus_two.binary_star(str, ctx, error_callback);
  auto* result_4 = minus_one.binary_star(str, ctx, error_callback);
  auto* result_5 = big.binary_star(str0, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "abcabc");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "abcabcabc");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "");
}

TEST(StarlarkString, BinaryStarNotInt) {
  starlark_string str("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = str.binary_star(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkString, BinaryStarTooBig) {
  starlark_string str("abc"sv);
  starlark_bigint big(number::one() << 64);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = str.binary_star(big, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 2147483647 elements");
}

TEST(StarlarkString, StarEqualsAssign) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_string str0(""sv);
  starlark_string str("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = str.star_equals_assign(two, ctx, error_callback);
  auto* result_2 = str.star_equals_assign(three, ctx, error_callback);
  auto* result_3 = str.star_equals_assign(minus_two, ctx, error_callback);
  auto* result_4 = str.star_equals_assign(minus_one, ctx, error_callback);
  auto* result_5 = str0.star_equals_assign(big, ctx, error_callback);
  auto* result_6 = str0.star_equals_assign(two, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "abcabc");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "abcabcabc");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "");
  ASSERT_NE(result_6, nullptr);
  EXPECT_EQ(result_6->str(), "");
}

TEST(StarlarkString, StarEqualsAssignReverse) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_string str0(""sv);
  starlark_string str("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = two.star_equals_assign(str, ctx, error_callback);
  auto* result_2 = three.star_equals_assign(str, ctx, error_callback);
  auto* result_3 = minus_two.star_equals_assign(str, ctx, error_callback);
  auto* result_4 = minus_one.star_equals_assign(str, ctx, error_callback);
  auto* result_5 = big.star_equals_assign(str0, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "abcabc");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "abcabcabc");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "");
}

TEST(StarlarkString, StarEqualsAssignNotInt) {
  starlark_string str("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = str.star_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkString, Len) {
  error_handler error_callback;

  EXPECT_EQ(0, starlark_string(""sv).len(true, error_callback));
  EXPECT_EQ(3, starlark_string("abc"sv).len(true, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkString, Index) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  EXPECT_EQ(str.index(starlark_integer(-3), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(str.index(starlark_integer(-2), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(str.index(starlark_integer(-1), ctx, error_callback)->repr(), "\"c\"");
  EXPECT_EQ(str.index(starlark_integer(0), ctx, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(str.index(starlark_integer(1), ctx, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(str.index(starlark_integer(2), ctx, error_callback)->repr(), "\"c\"");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkString, IndexOutOfRange1) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  EXPECT_EQ(nullptr, str.index(starlark_integer(-4), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("IndexError: string index out of range", error_callback.messages[0]);
}

TEST(StarlarkString, IndexOutOfRange2) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  EXPECT_EQ(nullptr, str.index(starlark_integer(3), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("IndexError: string index out of range", error_callback.messages[0]);
}

TEST(StarlarkString, Dir) {
  starlark_string str("abc"sv);

  EXPECT_THAT(str.dir(), Contains("capitalize"));
}

TEST(StarlarkString, Count) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abc"sv);

  auto* result = str.get_attr(true, "count", ctx, error_callback);
  ASSERT_NE(nullptr, result) << error_callback.messages[0];
  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* value = result->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, value);
  EXPECT_EQ(value->str(), "3");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkString, SliceRange) {
  Arena arena;
  context ctx(arena);
  auto test = [&ctx](const starlark_obj* start, const starlark_obj* end, const starlark_obj* stride,
      std::string_view expected_value0,
      std::string_view expected_value1,
      std::string_view expected_value2,
      std::string_view expected_value3,
      std::string_view expected_value4,
      std::string_view expected_value5) {
    error_handler error_callback;
    starlark_string str0(""sv);
    starlark_string str1("a"sv);
    starlark_string str2("ab"sv);
    starlark_string str3("abc"sv);
    starlark_string str4("abcd"sv);
    starlark_string str5("abcde"sv);

    auto* result0 = str0.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result1 = str1.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result2 = str2.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result3 = str3.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result4 = str4.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result5 = str5.slice_range(*start, *end, *stride, ctx, error_callback);

    ASSERT_NE(nullptr, result0);
    ASSERT_NE(nullptr, result1);
    ASSERT_NE(nullptr, result2);
    ASSERT_NE(nullptr, result3);
    ASSERT_NE(nullptr, result4);
    ASSERT_NE(nullptr, result5);
    EXPECT_EQ(expected_value0, result0->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value1, result1->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value2, result2->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value3, result3->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value4, result4->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_EQ(expected_value5, result5->str()) << "Start: " << start->str() << ", end: " << end->str() << ", stride: " << stride->str() << "\n";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  /*
  ```python
  def tt(a):
      if a == None:
          return "ctx.none_value()"
      if a == -1:
          return "ctx.minus_one()"
      if a == 0:
          return "ctx.zero()"
      if a == 1:
          return "ctx.one()"

  def rr(a, b, c):
      return 'test({}, {}, {}, "{}", "{}", "{}", "{}", "{}", "{}");'.format(tt(a), tt(b), tt(c), *[str('abcde'[:x][a:b:c]) for x in range(6)])
  "\n  ".join([rr(a,b,c) for a in (None, -1, 0, 1) for b in (None, -1, 0, 1) for c in (None, -1, 1)])
  ```
  */

  test(ctx.none_value(), ctx.none_value(), ctx.none_value(), "", "a", "ab", "abc", "abcd", "abcde");
  test(ctx.none_value(), ctx.none_value(), ctx.minus_one(), "", "a", "ba", "cba", "dcba", "edcba");
  test(ctx.none_value(), ctx.none_value(), ctx.one(), "", "a", "ab", "abc", "abcd", "abcde");
  test(ctx.none_value(), ctx.minus_one(), ctx.none_value(), "", "", "a", "ab", "abc", "abcd");
  test(ctx.none_value(), ctx.minus_one(), ctx.minus_one(), "", "", "", "", "", "");
  test(ctx.none_value(), ctx.minus_one(), ctx.one(), "", "", "a", "ab", "abc", "abcd");
  test(ctx.none_value(), ctx.zero(), ctx.none_value(), "", "", "", "", "", "");
  test(ctx.none_value(), ctx.zero(), ctx.minus_one(), "", "", "b", "cb", "dcb", "edcb");
  test(ctx.none_value(), ctx.zero(), ctx.one(), "", "", "", "", "", "");
  test(ctx.none_value(), ctx.one(), ctx.none_value(), "", "a", "a", "a", "a", "a");
  test(ctx.none_value(), ctx.one(), ctx.minus_one(), "", "", "", "c", "dc", "edc");
  test(ctx.none_value(), ctx.one(), ctx.one(), "", "a", "a", "a", "a", "a");
  test(ctx.minus_one(), ctx.none_value(), ctx.none_value(), "", "a", "b", "c", "d", "e");
  test(ctx.minus_one(), ctx.none_value(), ctx.minus_one(), "", "a", "ba", "cba", "dcba", "edcba");
  test(ctx.minus_one(), ctx.none_value(), ctx.one(), "", "a", "b", "c", "d", "e");
  test(ctx.minus_one(), ctx.minus_one(), ctx.none_value(), "", "", "", "", "", "");
  test(ctx.minus_one(), ctx.minus_one(), ctx.minus_one(), "", "", "", "", "", "");
  test(ctx.minus_one(), ctx.minus_one(), ctx.one(), "", "", "", "", "", "");
  test(ctx.minus_one(), ctx.zero(), ctx.none_value(), "", "", "", "", "", "");
  test(ctx.minus_one(), ctx.zero(), ctx.minus_one(), "", "", "b", "cb", "dcb", "edcb");
  test(ctx.minus_one(), ctx.zero(), ctx.one(), "", "", "", "", "", "");
  test(ctx.minus_one(), ctx.one(), ctx.none_value(), "", "a", "", "", "", "");
  test(ctx.minus_one(), ctx.one(), ctx.minus_one(), "", "", "", "c", "dc", "edc");
  test(ctx.minus_one(), ctx.one(), ctx.one(), "", "a", "", "", "", "");
  test(ctx.zero(), ctx.none_value(), ctx.none_value(), "", "a", "ab", "abc", "abcd", "abcde");
  test(ctx.zero(), ctx.none_value(), ctx.minus_one(), "", "a", "a", "a", "a", "a");
  test(ctx.zero(), ctx.none_value(), ctx.one(), "", "a", "ab", "abc", "abcd", "abcde");
  test(ctx.zero(), ctx.minus_one(), ctx.none_value(), "", "", "a", "ab", "abc", "abcd");
  test(ctx.zero(), ctx.minus_one(), ctx.minus_one(), "", "", "", "", "", "");
  test(ctx.zero(), ctx.minus_one(), ctx.one(), "", "", "a", "ab", "abc", "abcd");
  test(ctx.zero(), ctx.zero(), ctx.none_value(), "", "", "", "", "", "");
  test(ctx.zero(), ctx.zero(), ctx.minus_one(), "", "", "", "", "", "");
  test(ctx.zero(), ctx.zero(), ctx.one(), "", "", "", "", "", "");
  test(ctx.zero(), ctx.one(), ctx.none_value(), "", "a", "a", "a", "a", "a");
  test(ctx.zero(), ctx.one(), ctx.minus_one(), "", "", "", "", "", "");
  test(ctx.zero(), ctx.one(), ctx.one(), "", "a", "a", "a", "a", "a");
  test(ctx.one(), ctx.none_value(), ctx.none_value(), "", "", "b", "bc", "bcd", "bcde");
  test(ctx.one(), ctx.none_value(), ctx.minus_one(), "", "a", "ba", "ba", "ba", "ba");
  test(ctx.one(), ctx.none_value(), ctx.one(), "", "", "b", "bc", "bcd", "bcde");
  test(ctx.one(), ctx.minus_one(), ctx.none_value(), "", "", "", "b", "bc", "bcd");
  test(ctx.one(), ctx.minus_one(), ctx.minus_one(), "", "", "", "", "", "");
  test(ctx.one(), ctx.minus_one(), ctx.one(), "", "", "", "b", "bc", "bcd");
  test(ctx.one(), ctx.zero(), ctx.none_value(), "", "", "", "", "", "");
  test(ctx.one(), ctx.zero(), ctx.minus_one(), "", "", "b", "b", "b", "b");
  test(ctx.one(), ctx.zero(), ctx.one(), "", "", "", "", "", "");
  test(ctx.one(), ctx.one(), ctx.none_value(), "", "", "", "", "", "");
  test(ctx.one(), ctx.one(), ctx.minus_one(), "", "", "", "", "", "");
  test(ctx.one(), ctx.one(), ctx.one(), "", "", "", "", "", "");
}

TEST(StarlarkString, SliceRangeBoolStart) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abcdef"sv);

  auto* result = str.slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("abcdef", str.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkString, SliceRangeBoolEnd) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abcdef"sv);

  auto* result = str.slice_range(*ctx.none_value(), *ctx.false_value(), *ctx.none_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("abcdef", str.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkString, SliceRangeBoolStride) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abcdef"sv);

  auto* result = str.slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.false_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("abcdef", str.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkString, SliceRangeZeroStride) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_string str("abcdef"sv);

  auto* result = str.slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.zero(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("abcdef", str.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: slice step cannot be zero");
}

TEST(StarlarkString, InterpolationNoInterpolation) {
  auto test = [](std::string_view to_interpolate, std::string_view expected) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    starlark_string str(to_interpolate);
    starlark_tuple tuple(0);

    auto* result = str.binary_percent(tuple, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::string_t);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "");
  test("abc", "abc");
  test("%%", "%");
}

TEST(StarlarkString, Interpolation) {
  auto test = [](std::string_view to_interpolate, starlark_obj* input, std::string_view expected) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    starlark_string str(to_interpolate);

    auto* result1 = str.binary_percent(*input, ctx, error_callback);
    auto* result2 = str.percent_equals_assign(*input, ctx, error_callback);
    ASSERT_NE(nullptr, result1);
    ASSERT_NE(nullptr, result2);
    EXPECT_EQ(result1->type(), starlark_types::string_t);
    EXPECT_EQ(result1->str(), expected);
    EXPECT_EQ(result2->type(), starlark_types::string_t);
    EXPECT_EQ(result2->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };
  auto test_with_error = [](std::string_view to_interpolate, starlark_obj* input, std::string_view expected_error) {
    Arena arena;
    context ctx(arena);
    error_handler error_callback;
    starlark_string str(to_interpolate);

    auto* result = str.binary_percent(*input, ctx, error_callback);
    ASSERT_EQ(nullptr, result);
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], expected_error);
  };
  starlark_integer one(1);
  starlark_integer two(2);
  starlark_tuple tuple_one_two(2);
  tuple_one_two.add(&one);
  tuple_one_two.add(&two);
  starlark_tuple tuple(0);
  starlark_tuple tuple_tuple(1);
  tuple_tuple.add(&tuple);
  starlark_integer small_int(123);
  starlark_bigint big_int(parse_number("123456789012345678901234567890", nullptr, 0));
  starlark_bigint huge_int(number::one() << 2000);
  starlark_float big_float(1e100);
  starlark_float plus_inf(std::strtod("+inf", nullptr));
  starlark_float minus_inf(std::strtod("-inf", nullptr));
  starlark_float plus_nan(std::strtod("+nan", nullptr));
  starlark_float minus_nan(std::strtod("-nan", nullptr));
  starlark_string small_string("abc"sv);
  starlark_list list_one_two(2);
  error_handler error_callback;
  list_one_two.append(&one, error_callback);
  list_one_two.append(&two, error_callback);

  test("%s", &one, "1");
  test("abc%sdef", &one, "abc1def");
  test("abc%s%sdef", &tuple_one_two, "abc12def");
  test("abc%sdef%sghi", &tuple_one_two, "abc1def2ghi");
  test(" XXX %r XXX", &tuple_tuple, " XXX () XXX");

  test("%d", &small_int, "123");
  test("%d", &big_int, "123456789012345678901234567890");
  test("%d", &huge_int, "114813069527425452423283320117768198402231770208869520047764273682576626139237031385665948631650626991844596463898746277344711896086305533142593135616665318539129989145312280000688779148240044871428926990063486244781615463646388363947317026040466353970904996558162398808944629605623311649536164221970332681344168908984458505602379484807914058900934776500429002716706625830522008132236281291761267883317206598995396418127021779858404042159853183251540889433902091920554957783589672039160081957216630582755380425583726015528348786419432054508915275783882625175435528800822842770817965453762184851149029376");
  test("%d", & big_float, "10000000000000000159028911097599180468360808563945281389781327557747838772170381060813469985856815104");
  test_with_error("%d", &plus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%d", &minus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%d", &plus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%d", &minus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%d", &small_string, "TypeError: %d format: an integer is required, not string");

  test("%o", &small_int, "173");
  test("%o", &big_int, "143564417755415637016711617605322");
  test("%o", &huge_int, "4000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
  test("%o", & big_float, "444465511312303372000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
  test_with_error("%o", &plus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%o", &minus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%o", &plus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%o", &minus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%o", &small_string, "TypeError: %o format: an integer is required, not string");

  test("%x", &small_int, "7b");
  test("%x", &big_int, "18ee90ff6c373e0ee4e3f0ad2");
  test("%x", &huge_int, "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
  test("%x", & big_float, "1249ad2594c37d0000000000000000000000000000000000000000000000000000000000000000000000");
  test_with_error("%x", &plus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%x", &minus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%x", &plus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%x", &minus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%x", &small_string, "TypeError: %x format: an integer is required, not string");

  test("%X", &small_int, "7B");
  test("%X", &big_int, "18EE90FF6C373E0EE4E3F0AD2");
  test("%X", &huge_int, "100000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000");
  test("%X", & big_float, "1249AD2594C37D0000000000000000000000000000000000000000000000000000000000000000000000");
  test_with_error("%X", &plus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%X", &minus_inf, "OverflowError: cannot convert float infinity to integer");
  test_with_error("%X", &plus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%X", &minus_nan, "ValueError: cannot convert float NaN to integer");
  test_with_error("%X", &small_string, "TypeError: %X format: an integer is required, not string");

  test("%e", &small_int, "1.230000e+02");
  test("%e", &big_int, "1.234568e+29");
  test("%e", &huge_int, "inf");
  test("%e", & big_float, "1.000000e+100");
  test("%e", &plus_inf, "inf");
  test("%e", &minus_inf, "-inf");
  test("%e", &plus_nan, "nan");
  test("%e", &minus_nan, "nan");
  test_with_error("%e", &small_string, "TypeError: %e format: a real number is required, not string");

  test("%E", &small_int, "1.230000E+02");
  test("%E", &big_int, "1.234568E+29");
  test("%E", &huge_int, "inf");
  test("%E", & big_float, "1.000000E+100");
  test("%E", &plus_inf, "inf");
  test("%E", &minus_inf, "-inf");
  test("%E", &plus_nan, "nan");
  test("%E", &minus_nan, "nan");
  test_with_error("%E", &small_string, "TypeError: %E format: a real number is required, not string");

  test("%f", &small_int, "123.000000");
  test("%f", &big_int, "123456789012345677877719597056.000000");
  test("%f", &huge_int, "inf");
  test("%f", & big_float, "10000000000000000159028911097599180468360808563945281389781327557747838772170381060813469985856815104.000000");
  test("%f", &plus_inf, "inf");
  test("%f", &minus_inf, "-inf");
  test("%f", &plus_nan, "nan");
  test("%f", &minus_nan, "nan");
  test_with_error("%f", &small_string, "TypeError: %f format: a real number is required, not string");

  test("%F", &small_int, "123.000000");
  test("%F", &big_int, "123456789012345677877719597056.000000");
  test("%F", &huge_int, "inf");
  test("%F", & big_float, "10000000000000000159028911097599180468360808563945281389781327557747838772170381060813469985856815104.000000");
  test("%F", &plus_inf, "inf");
  test("%F", &minus_inf, "-inf");
  test("%F", &plus_nan, "nan");
  test("%F", &minus_nan, "nan");
  test_with_error("%F", &small_string, "TypeError: %F format: a real number is required, not string");

  test("%g", &small_int, "123.0");
  test("%g", &big_int, "1.2345678901234568e+29");
  test("%g", &huge_int, "inf");
  test("%g", & big_float, "1e+100");
  test("%g", &plus_inf, "inf");
  test("%g", &minus_inf, "-inf");
  test("%g", &plus_nan, "nan");
  test("%g", &minus_nan, "nan");
  test_with_error("%g", &small_string, "TypeError: %g format: a real number is required, not string");

  test("%G", &small_int, "123.0");
  test("%G", &big_int, "1.2345678901234568E+29");
  test("%G", &huge_int, "inf");
  test("%G", & big_float, "1E+100");
  test("%G", &plus_inf, "inf");
  test("%G", &minus_inf, "-inf");
  test("%G", &plus_nan, "nan");
  test("%G", &minus_nan, "nan");
  test_with_error("%G", &small_string, "TypeError: %G format: a real number is required, not string");


  test_with_error("%", &one, "ValueError: incomplete format");
  test_with_error("%w", &one, "ValueError: unsupported format character 'w' (0x77) at index 1");
  test_with_error("%w %w", &tuple_one_two, "ValueError: unsupported format character 'w' (0x77) at index 1");
  test_with_error("%d", &tuple, "TypeError: not enough arguments for format string");
  test_with_error("%d", &tuple_one_two, "TypeError: not all arguments converted during string formatting");
  test_with_error("%d %d", &list_one_two, "TypeError: not enough arguments for format string");
}

}  // namespace
