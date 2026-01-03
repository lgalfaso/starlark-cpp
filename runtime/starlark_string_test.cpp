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
#include "runtime/starlark_struct.hpp"
#include "runtime/starlark_testing.hpp"
#include "runtime/starlark_tuple.hpp"

using ::google::protobuf::Arena;
using ::starlark::bigint::number;
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
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_struct;
using ::starlark::runtime::starlark_tuple;
using ::starlark::testing::error_handler;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

TEST(StarlarkString, Type) {
  EXPECT_EQ("string", starlark_string("").type());
}

TEST(StarlarkString, Primitve) {
  EXPECT_TRUE(starlark_string("").primitive());
}

TEST(StarlarkString, Str) {
  EXPECT_EQ("abcdef", starlark_string("abcdef").str());
  EXPECT_EQ("fedcba", starlark_string("fedcba").str());
}

TEST(StarlarkString, Repr) {
  // TODO(lmirelmann): Would be nice to have a test that checks the encoding of all characters.
  EXPECT_EQ("\"abcdef\"", starlark_string("abcdef").repr());
  EXPECT_EQ("\"'\"", starlark_string("'").repr());
  EXPECT_EQ("\"'\\\"\"", starlark_string("'\"").repr());
  EXPECT_EQ("\"\\t\\r\\n\"", starlark_string("\t\r\n").repr());
  EXPECT_EQ("\"\\x01\\x02\\x7f\"", starlark_string("\001\002\177").repr());
  EXPECT_EQ("\"\\x90\"", starlark_string("\302\220").repr());
  EXPECT_EQ("\"\xC3\xA0\"", starlark_string("\303\240").repr());
  EXPECT_EQ("\"\xC8\xB4\"", starlark_string("\310\264").repr());
  EXPECT_EQ("\"\\u0378\"", starlark_string("\315\270").repr());
  EXPECT_EQ("\"\\ud800\"", starlark_string("\355\240\200").repr());
  EXPECT_EQ("\"\\U000101c7\"", starlark_string("\360\220\207\207").repr());
  EXPECT_EQ("\"\xf0\"", starlark_string(std::string("🙂").substr(0, 1)).repr());
  EXPECT_EQ("\"\\ufeff\"", starlark_string("\xef\xbb\xbf").repr());
}

TEST(StarlarkString, Truthy) {
  EXPECT_FALSE(starlark_string("").truthy());
  EXPECT_TRUE(starlark_string("a").truthy());
}

TEST(StarlarkString, Equals) {
  EXPECT_TRUE(starlark_string("").equals(starlark_string("")));
  EXPECT_TRUE(starlark_string("a").equals(starlark_string("a")));
  EXPECT_FALSE(starlark_string("").equals(starlark_string("a")));
  // This is the NFKC decomposition.
  EXPECT_FALSE(starlark_string("\u03C9\u0301").equals(starlark_string("\u03CE")));

  EXPECT_FALSE(starlark_string("").equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_string("").equals(starlark_bool(false)));
  EXPECT_FALSE(starlark_string("").equals(starlark_bytes("")));
  EXPECT_FALSE(starlark_string("").equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_string("").equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_string("").equals(starlark_function()));
  EXPECT_FALSE(starlark_string("").equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_string("").equals(starlark_list(0)));
  EXPECT_FALSE(starlark_string("").equals(starlark_none()));
  EXPECT_FALSE(starlark_string("").equals(starlark_range(0, 1, 1)));
  EXPECT_FALSE(starlark_string("").equals(starlark_set()));
  EXPECT_FALSE(starlark_string("").equals(starlark_struct()));
  EXPECT_FALSE(starlark_string("").equals(starlark_tuple(0)));

  EXPECT_FALSE(starlark_string("0").equals(starlark_integer(0)));
}

TEST(StarlarkString, Hash) {
  EXPECT_EQ(0, starlark_string("").hash());
  EXPECT_EQ(0x539375b79c9167a6, starlark_string(std::string("\000", 1)).hash());
  EXPECT_EQ(0x19ac88fb2429e222, starlark_string("a").hash());
  EXPECT_EQ(0x12d48511de046bbc, starlark_string("ab").hash());
  EXPECT_EQ(0x53d7cad63d3282ff, starlark_string("abc").hash());
  EXPECT_EQ(0x92981e87e70229, starlark_string("abcd").hash());
  EXPECT_EQ(0xabac8f555b5e912, starlark_string("abcde").hash());
  EXPECT_EQ(0x63667dc70d4dbf89, starlark_string("abcdef").hash());
  EXPECT_EQ(0x3539f9b81c64a336, starlark_string("abcdefg").hash());
  EXPECT_EQ(-0x7f95eee8bc7b632d, starlark_string("abcdefgh").hash());
  EXPECT_EQ(-0x642099e9fba65ad0, starlark_string("abcdefghi").hash());
  EXPECT_EQ(-0x669943fc9f1993d5, starlark_string("abcdefghij").hash());
  EXPECT_EQ(0x5e5235ef5071d1c8, starlark_string("abcdefghijk").hash());
  EXPECT_EQ(-0x574fd3e31dd1b6c0, starlark_string("abcdefghijkl").hash());
  EXPECT_EQ(0x348e1f9d6e3c67a9, starlark_string("abcdefghijklm").hash());
  EXPECT_EQ(0x4b59cf75df0ea525, starlark_string("abcdefghijklmn").hash());
  EXPECT_EQ(0x7c0d99416b677716, starlark_string("abcdefghijklmno").hash());
  EXPECT_EQ(0x651dcfdc9b304273, starlark_string("abcdefghijklmnop").hash());
  EXPECT_EQ(-0x706278fdd6f9101, starlark_string("abcdefghijklmnopq").hash());
  EXPECT_EQ(0x310a2e3dd291e1a6, starlark_string("abcdefghijklmnopqr").hash());
  EXPECT_EQ(0x76e0c0d29cc59331, starlark_string("abcdefghijklmnopqrs").hash());
  EXPECT_EQ(-0x89ec229ad3ce95e, starlark_string("abcdefghijklmnopqrst").hash());
  EXPECT_EQ(-0x3421666900cc950, starlark_string("abcdefghijklmnopqrstu").hash());
  EXPECT_EQ(0x7be37d6337827631, starlark_string("abcdefghijklmnopqrstuv").hash());
  EXPECT_EQ(-0xd93501f3830d3c5, starlark_string("abcdefghijklmnopqrstuvw").hash());
  EXPECT_EQ(-0x1ec56e52deeb2ed9, starlark_string("abcdefghijklmnopqrstuvwx").hash());
  EXPECT_EQ(0x2928ab0caeae7b0d, starlark_string("abcdefghijklmnopqrstuvwxy").hash());
  EXPECT_EQ(0x732f1d3705b6dfa3, starlark_string("abcdefghijklmnopqrstuvwxyz").hash());
  EXPECT_EQ(-0x3d305c344304b1, starlark_string("abcdefghijklmnopqrstuvwxyz0").hash());
  EXPECT_EQ(-0x4bbc24288075d499, starlark_string("abcdefghijklmnopqrstuvwxyz01").hash());
  EXPECT_EQ(0x630596a3267cd0f9, starlark_string("abcdefghijklmnopqrstuvwxyz012").hash());
  EXPECT_EQ(-0x7176a79e04f34841, starlark_string("abcdefghijklmnopqrstuvwxyz0123").hash());
  EXPECT_EQ(0x7b3b18b2511d145a, starlark_string("abcdefghijklmnopqrstuvwxyz01234").hash());
  EXPECT_EQ(-0x116d8bb10196190f, starlark_string("abcdefghijklmnopqrstuvwxyz012345").hash());
  EXPECT_EQ(-0x7a27473192c1723a, starlark_string("abcdefghijklmnopqrstuvwxyz0123456").hash());
  EXPECT_EQ(0x731309efc4441b64, starlark_string("abcdefghijklmnopqrstuvwxyz01234567").hash());
  EXPECT_EQ(-0x41044b627d75b155, starlark_string("abcdefghijklmnopqrstuvwxyz012345678").hash());
  EXPECT_EQ(-0x38976cda9bea7d32, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789").hash());
  EXPECT_EQ(0x6079104dbaf33098, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789@").hash());
  EXPECT_EQ(-0x6f9048eb54a00388, starlark_string("abcdefghijklmnopqrstuvwxyz0123456789@!").hash());
}

TEST(StarlarkString, Order) {
  error_handler error_callback;

  EXPECT_THAT(starlark_string("").cmp(starlark_string(""), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_string("").cmp(starlark_string("a"), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_string("a").cmp(starlark_string("a"), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_string("a").cmp(starlark_string(""), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_string("a").cmp(starlark_string("b"), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_string("b").cmp(starlark_string("a"), "cmp", error_callback), Gt(0));
}

TEST(StarlarkString, OrderErrors) {
  error_handler error_callback;
  EXPECT_FALSE(starlark_string("").cmp(starlark_bytes(""), "<", error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'string' and 'bytes'");
}

TEST(StarlarkString, BinaryIn) {
  error_handler error_callback;
  EXPECT_TRUE(starlark_string("").binary_in(starlark_string(""), error_callback));
  EXPECT_TRUE(starlark_string("a").binary_in(starlark_string(""), error_callback));
  EXPECT_FALSE(starlark_string("a").binary_in(starlark_string("b"), error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkString, BinaryInErrors) {
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string("a").binary_in(starlark_integer('b'), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string("a").binary_in(starlark_integer('a'), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string("a").binary_in(starlark_bigint('a'), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_string(""), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: a bytes-like object is required, not 'string'");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_integer(-1), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_integer(256), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_bigint(-1), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_bigint(256), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
}

TEST(StarlarkString, BinaryPlus) {
  starlark_string str_1("abc");
  starlark_string str_2("def");
  Arena arena;
  error_handler error_callback;

  auto* result = str_1.binary_plus(str_2, arena, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "abcdef");
}

TEST(StarlarkString, BinaryPlusNotList) {
  starlark_string str("abc");
  starlark_tuple tuple(0);
  Arena arena;
  error_handler error_callback;

  auto* result = str.binary_plus(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't concat tuple to string");
}

TEST(StarlarkString, BinaryStar) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one << 64);
  starlark_string str0("");
  starlark_string str("abc");
  Arena arena;
  error_handler error_callback;

  auto* result_1 = str.binary_star(two, arena, error_callback);
  auto* result_2 = str.binary_star(three, arena, error_callback);
  auto* result_3 = str.binary_star(minus_two, arena, error_callback);
  auto* result_4 = str.binary_star(minus_one, arena, error_callback);
  auto* result_5 = str0.binary_star(big, arena, error_callback);
  auto* result_6 = str0.binary_star(two, arena, error_callback);

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
  starlark_bigint big(number::one << 64);
  starlark_string str0("");
  starlark_string str("abc");
  Arena arena;
  error_handler error_callback;

  auto* result_1 = two.binary_star(str, arena, error_callback);
  auto* result_2 = three.binary_star(str, arena, error_callback);
  auto* result_3 = minus_two.binary_star(str, arena, error_callback);
  auto* result_4 = minus_one.binary_star(str, arena, error_callback);
  auto* result_5 = big.binary_star(str0, arena, error_callback);

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
  starlark_string str("abc");
  starlark_tuple tuple(0);
  Arena arena;
  error_handler error_callback;

  auto* result = str.binary_star(tuple, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkString, BinaryStarTooBig) {
  starlark_string str("abc");
  starlark_bigint big(number::one << 64);
  Arena arena;
  error_handler error_callback;

  auto* result = str.binary_star(big, arena, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 2147483647 elements");
}

TEST(StarlarkString, Len) {
  error_handler error_callback;

  EXPECT_EQ(0, starlark_string("").len(true, error_callback));
  EXPECT_EQ(3, starlark_string("abc").len(true, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkString, Index) {
  error_handler error_callback;
  Arena arena;
  starlark_string str("abc");

  EXPECT_EQ(str.index(starlark_integer(-3), arena, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(str.index(starlark_integer(-2), arena, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(str.index(starlark_integer(-1), arena, error_callback)->repr(), "\"c\"");
  EXPECT_EQ(str.index(starlark_integer(0), arena, error_callback)->repr(), "\"a\"");
  EXPECT_EQ(str.index(starlark_integer(1), arena, error_callback)->repr(), "\"b\"");
  EXPECT_EQ(str.index(starlark_integer(2), arena, error_callback)->repr(), "\"c\"");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkString, IndexOutOfRange1) {
  error_handler error_callback;
  Arena arena;
  starlark_string str("abc");

  EXPECT_EQ(nullptr, str.index(starlark_integer(-4), arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("IndexError: string index out of range", error_callback.messages[0]);
}

TEST(StarlarkString, IndexOutOfRange2) {
  error_handler error_callback;
  Arena arena;
  starlark_string str("abc");

  EXPECT_EQ(nullptr, str.index(starlark_integer(3), arena, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("IndexError: string index out of range", error_callback.messages[0]);
}

}  // namespace
