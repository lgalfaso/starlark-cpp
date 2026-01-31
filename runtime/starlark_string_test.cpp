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

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
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

}  // namespace
