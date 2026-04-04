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
using ::starlark::runtime::error_fn;
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

class starlark_testing_function : public starlark_function {
 public:
  starlark_obj* call(const starlark_obj::pos_args_t& pos_args, const starlark_obj::named_args_t& named_args, context& ctx, error_fn& error_callback) override {
    return starlark_obj::call(pos_args, named_args, ctx, error_callback);
  }
};

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
  EXPECT_FALSE(starlark_string(""sv).equals(starlark_testing_function()));
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
    EXPECT_FALSE(starlark_string(""sv).binary_in(starlark_bytes(""sv), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not bytes");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string(""sv).binary_in(starlark_integer(-1), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string(""sv).binary_in(starlark_integer(256), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string(""sv).binary_in(starlark_bigint(-1), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_string(""sv).binary_in(starlark_bigint(256), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: 'in <string>' requires string as left operand, not int");
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

TEST(StarlarkString, CountNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count expected at least 1 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, CountEmptyString) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_string empty(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&empty);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "4");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, Count) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountOverlap) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("aaaaaa"sv);
  starlark_string a_str("aa"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "aaaaaa");
}

TEST(StarlarkString, CountInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_integer a_int('a');

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_int);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not int");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountBigInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_bigint a_int('a');

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_int);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not int");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountIntegerNegative) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not int");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountIntegerTooBig) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_integer some_int(256);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&some_int);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not int");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountBigIntNegative) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_bigint some_int(-1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&some_int);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not int");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountBigIntTooBig) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_bigint some_int(256);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&some_int);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not int");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.none_value());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not NoneType");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountFloat) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_float f_value('a');

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&f_value);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count() argument 1 must be string, not float");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountTwoArgumentsStartInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("b"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "1");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountTwoArgumentsStartIntSkip) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("b"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.one());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountTwoArgumentsStartNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.none_value());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountTwoArgumentsStartBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.false_value());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountThreeArgumentsStartInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("a"sv);
  starlark_integer some_int(100);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&some_int);
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountThreeArgumentsEndIntSkip) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.one());
  pos_args.push_back(ctx.minus_one());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "2");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountThreeArgumentsEndNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.none_value());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountThreeArgumentsEndBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string a_str("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_str);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.false_value());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, CountEmptyStringSameStartAndEnd) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_string empty(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&empty);
  pos_args.push_back(ctx.one());
  pos_args.push_back(ctx.one());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "1");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, CountEmptyStringEndBeforeStart) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_string empty(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&empty);
  pos_args.push_back(ctx.one());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, CountFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, CountWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.count() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: endswith expected at least 1 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithOneArgument) {
  auto test = [](std::string_view entry, std::string_view ending, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(entry);
    starlark_string param(ending);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param);
    auto* method = str.dot("endswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_THAT(error_callback.messages, IsEmpty());
    EXPECT_EQ(result->truthy(), expected);
  };

  test("", "", true);
  test("", "a", false);
  test("abc", "c", true);
  test("abc", "abc", true);
  test("abc", "", true);
  test("abc", "a", false);
  test("abc", "aabc", false);
}

TEST(StarlarkString, EndswithEmptyTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_FALSE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithTupleWithOneEntry) {
  auto test = [](std::string_view element, std::string_view entry, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_tuple tuple(1);
    starlark_string param(entry);
    tuple.add(&param);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&tuple);
    auto* method = str.dot("endswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->truthy(), expected);
  };

  test("", "", true);
  test("", "a", false);
  test("abc", "c", true);
  test("abc", "abc", true);
  test("abc", "", true);
  test("abc", "a", false);
  test("abc", "aabc", false);
}

TEST(StarlarkString, EndswithBytes) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_bytes bytes(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: endswith() argument 1 must be string, not bytes");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithTupleWithBytes) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_tuple tuple(1);
  starlark_bytes bytes(""sv);
  tuple.add(&bytes);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: tuple for endswith must only contain string, not bytes");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithTwoArguments) {
  auto test = [](std::string_view entry, std::string_view ending, int64_t start, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(entry);
    starlark_string param1(ending);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = str.dot("endswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_THAT(error_callback.messages, IsEmpty());
    EXPECT_EQ(result->truthy(), expected) << "Entry: " << entry << ", ending: " << ending << ", start: " << start;
  };

  test("", "", 0, true);
  test("", "", 1, false);
  test("", "a", 0, false);
  test("", "a", 1, false);
  test("abc", "c", 0, true);
  test("abc", "c", 2, true);
  test("abc", "c", 3, false);
  test("abc", "abc", 0, true);
  test("abc", "abc", 1, false);
  test("abc", "abc", -1, false);
  test("abc", "abc", -2, false);
  test("abc", "abc", -3, true);
  test("abc", "abc", -4, true);
  test("abc", "", 0, true);
  test("abc", "", 2, true);
  test("abc", "", 3, true);
  test("abc", "", 4, false);
  test("abc", "a", 0, false);
  test("abc", "a", 3, false);
  test("abc", "aabc", -1, false);
  test("abc", "aabc", 0, false);
  test("abc", "aabc", 3, false);
}

TEST(StarlarkString, EndswithStartNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithStartNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.none_value());
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithThreeArguments) {
  auto test = [](std::string_view entry, std::string_view ending, int64_t start, int64_t end, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(entry);
    starlark_string param1(ending);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = str.dot("endswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_THAT(error_callback.messages, IsEmpty());
    EXPECT_EQ(result->truthy(), expected) << "Entry: b'" << entry << "', ending: b'" << ending << "', start: " << start << ", end: " << end;
  };

  test("", "", 0, -1, true);
  test("", "", 0, 0, true);
  test("", "", 1, 0, false);
  test("", "a", 0, 1, false);
  test("", "a", 1, 1, false);
  test("abc", "c", 0, 3, true);
  test("abc", "c", 2, 3, true);
  test("abc", "c", 3, 3, false);
  test("abc", "c", 1, 2, false);
  test("abc", "b", 1, 2, true);
  test("abc", "abc", 0, -1, false);
  test("abc", "ab", 0, -1, true);
  test("abc", "abc", 0, 2, false);
  test("abc", "abc", 0, 3, true);
  test("abc", "abc", 0, 4, true);
  test("abc", "abc", 1, 3, false);
  test("abc", "abc", -1, 4, false);
  test("abc", "abc", -2, 10, false);
  test("abc", "", 0, 3, true);
  test("abc", "", 2, 3, true);
  test("abc", "", 2, 1, false);
  test("abc", "", 3, 3, true);
  test("abc", "", 4, 3, false);
  test("abc", "a", 0, 3, false);
  test("abc", "a", 0, 1, true);
  test("abc", "a", 3, 3, false);
  test("abc", "aabc", -1, 2, false);
  test("abc", "aabc", 0, 4, false);
  test("abc", "aabc", 3, 3, false);

  /*
  for a in range(-5, 6):
      for b in range(-5, 6):
          print('  test("abab", "ab", {}, {}, {});'.format(a, b, str("abab".endswith("ab", a, b)).lower()))
  */
  test("abab", "ab", -5, -5, false);
  test("abab", "ab", -5, -4, false);
  test("abab", "ab", -5, -3, false);
  test("abab", "ab", -5, -2, true);
  test("abab", "ab", -5, -1, false);
  test("abab", "ab", -5, 0, false);
  test("abab", "ab", -5, 1, false);
  test("abab", "ab", -5, 2, true);
  test("abab", "ab", -5, 3, false);
  test("abab", "ab", -5, 4, true);
  test("abab", "ab", -5, 5, true);
  test("abab", "ab", -4, -5, false);
  test("abab", "ab", -4, -4, false);
  test("abab", "ab", -4, -3, false);
  test("abab", "ab", -4, -2, true);
  test("abab", "ab", -4, -1, false);
  test("abab", "ab", -4, 0, false);
  test("abab", "ab", -4, 1, false);
  test("abab", "ab", -4, 2, true);
  test("abab", "ab", -4, 3, false);
  test("abab", "ab", -4, 4, true);
  test("abab", "ab", -4, 5, true);
  test("abab", "ab", -3, -5, false);
  test("abab", "ab", -3, -4, false);
  test("abab", "ab", -3, -3, false);
  test("abab", "ab", -3, -2, false);
  test("abab", "ab", -3, -1, false);
  test("abab", "ab", -3, 0, false);
  test("abab", "ab", -3, 1, false);
  test("abab", "ab", -3, 2, false);
  test("abab", "ab", -3, 3, false);
  test("abab", "ab", -3, 4, true);
  test("abab", "ab", -3, 5, true);
  test("abab", "ab", -2, -5, false);
  test("abab", "ab", -2, -4, false);
  test("abab", "ab", -2, -3, false);
  test("abab", "ab", -2, -2, false);
  test("abab", "ab", -2, -1, false);
  test("abab", "ab", -2, 0, false);
  test("abab", "ab", -2, 1, false);
  test("abab", "ab", -2, 2, false);
  test("abab", "ab", -2, 3, false);
  test("abab", "ab", -2, 4, true);
  test("abab", "ab", -2, 5, true);
  test("abab", "ab", -1, -5, false);
  test("abab", "ab", -1, -4, false);
  test("abab", "ab", -1, -3, false);
  test("abab", "ab", -1, -2, false);
  test("abab", "ab", -1, -1, false);
  test("abab", "ab", -1, 0, false);
  test("abab", "ab", -1, 1, false);
  test("abab", "ab", -1, 2, false);
  test("abab", "ab", -1, 3, false);
  test("abab", "ab", -1, 4, false);
  test("abab", "ab", -1, 5, false);
  test("abab", "ab", 0, -5, false);
  test("abab", "ab", 0, -4, false);
  test("abab", "ab", 0, -3, false);
  test("abab", "ab", 0, -2, true);
  test("abab", "ab", 0, -1, false);
  test("abab", "ab", 0, 0, false);
  test("abab", "ab", 0, 1, false);
  test("abab", "ab", 0, 2, true);
  test("abab", "ab", 0, 3, false);
  test("abab", "ab", 0, 4, true);
  test("abab", "ab", 0, 5, true);
  test("abab", "ab", 1, -5, false);
  test("abab", "ab", 1, -4, false);
  test("abab", "ab", 1, -3, false);
  test("abab", "ab", 1, -2, false);
  test("abab", "ab", 1, -1, false);
  test("abab", "ab", 1, 0, false);
  test("abab", "ab", 1, 1, false);
  test("abab", "ab", 1, 2, false);
  test("abab", "ab", 1, 3, false);
  test("abab", "ab", 1, 4, true);
  test("abab", "ab", 1, 5, true);
  test("abab", "ab", 2, -5, false);
  test("abab", "ab", 2, -4, false);
  test("abab", "ab", 2, -3, false);
  test("abab", "ab", 2, -2, false);
  test("abab", "ab", 2, -1, false);
  test("abab", "ab", 2, 0, false);
  test("abab", "ab", 2, 1, false);
  test("abab", "ab", 2, 2, false);
  test("abab", "ab", 2, 3, false);
  test("abab", "ab", 2, 4, true);
  test("abab", "ab", 2, 5, true);
  test("abab", "ab", 3, -5, false);
  test("abab", "ab", 3, -4, false);
  test("abab", "ab", 3, -3, false);
  test("abab", "ab", 3, -2, false);
  test("abab", "ab", 3, -1, false);
  test("abab", "ab", 3, 0, false);
  test("abab", "ab", 3, 1, false);
  test("abab", "ab", 3, 2, false);
  test("abab", "ab", 3, 3, false);
  test("abab", "ab", 3, 4, false);
  test("abab", "ab", 3, 5, false);
  test("abab", "ab", 4, -5, false);
  test("abab", "ab", 4, -4, false);
  test("abab", "ab", 4, -3, false);
  test("abab", "ab", 4, -2, false);
  test("abab", "ab", 4, -1, false);
  test("abab", "ab", 4, 0, false);
  test("abab", "ab", 4, 1, false);
  test("abab", "ab", 4, 2, false);
  test("abab", "ab", 4, 3, false);
  test("abab", "ab", 4, 4, false);
  test("abab", "ab", 4, 5, false);
  test("abab", "ab", 5, -5, false);
  test("abab", "ab", 5, -4, false);
  test("abab", "ab", 5, -3, false);
  test("abab", "ab", 5, -2, false);
  test("abab", "ab", 5, -1, false);
  test("abab", "ab", 5, 0, false);
  test("abab", "ab", 5, 1, false);
  test("abab", "ab", 5, 2, false);
  test("abab", "ab", 5, 3, false);
  test("abab", "ab", 5, 4, false);
  test("abab", "ab", 5, 5, false);

  /*
  for a in range(-1, 2):
      for b in range(-1, 2):
          print('  test("", "", {}, {}, {});'.format(a, b, str("".endswith("", a, b)).lower()))
  */
  test("", "", -1, -1, true);
  test("", "", -1, 0, true);
  test("", "", -1, 1, true);
  test("", "", 0, -1, true);
  test("", "", 0, 0, true);
  test("", "", 0, 1, true);
  test("", "", 1, -1, false);
  test("", "", 1, 0, false);
  test("", "", 1, 1, false);
}

TEST(StarlarkString, EndswithEndNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithEndNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.none_value());
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: endswith expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, EndswithWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.endswith() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: startswith expected at least 1 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithOneArgument) {
  auto test = [](std::string_view entry, std::string_view beginning, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(entry);
    starlark_string param(beginning);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param);
    auto* method = str.dot("startswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_THAT(error_callback.messages, IsEmpty());
    EXPECT_EQ(result->truthy(), expected);
  };

  test("", "", true);
  test("", "a", false);
  test("abc", "a", true);
  test("abc", "abc", true);
  test("abc", "", true);
  test("abc", "c", false);
  test("abc", "abcc", false);
}

TEST(StarlarkString, StartswithEmptyTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_FALSE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithTupleWithOneEntry) {
  auto test = [](std::string_view element, std::string_view entry, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_tuple tuple(1);
    starlark_string param(entry);
    tuple.add(&param);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&tuple);
    auto* method = str.dot("startswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->truthy(), expected);
  };

  test("", "", true);
  test("", "a", false);
  test("abc", "a", true);
  test("abc", "abc", true);
  test("abc", "", true);
  test("abc", "c", false);
  test("abc", "abcc", false);
}

TEST(StarlarkString, StartswithBytes) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_bytes bytes(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: startswith() argument 1 must be string, not bytes");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithTupleWithBytes) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_tuple tuple(1);
  starlark_bytes bytes(""sv);
  tuple.add(&bytes);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: tuple for startswith must only contain string, not bytes");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithTwoArguments) {
  auto test = [](std::string_view entry, std::string_view beginning, int64_t start, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(entry);
    starlark_string param1(beginning);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = str.dot("startswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_THAT(error_callback.messages, IsEmpty());
    EXPECT_EQ(result->truthy(), expected) << "Entry: " << entry << ", beginning: " << beginning << ", start: " << start;
  };

  test("", "", 0, true);
  test("", "", 1, false);
  test("", "a", 0, false);
  test("", "a", 1, false);
  test("abc", "a", 0, true);
  test("abc", "a", 2, false);
  test("abc", "a", 3, false);
  test("abc", "abc", 0, true);
  test("abc", "abc", 1, false);
  test("abc", "abc", -1, false);
  test("abc", "abc", -2, false);
  test("abc", "abc", -3, true);
  test("abc", "abc", -4, true);
  test("abc", "", 0, true);
  test("abc", "", 2, true);
  test("abc", "", 3, true);
  test("abc", "", 4, false);
  test("abc", "c", 0, false);
  test("abc", "c", 2, true);
  test("abc", "c", 3, false);
  test("abc", "abcc", -1, false);
  test("abc", "abcc", 0, false);
  test("abc", "abcc", 3, false);
}

TEST(StarlarkString, StartswithStartNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithStartNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.none_value());
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithThreeArguments) {
  auto test = [](std::string_view entry, std::string_view beginning, int64_t start, int64_t end, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(entry);
    starlark_string param1(beginning);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = str.dot("startswith", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    ASSERT_THAT(error_callback.messages, IsEmpty());
    EXPECT_EQ(result->truthy(), expected) << "Entry: b'" << entry << "', beginning: b'" << beginning << "', start: " << start << ", end: " << end;
  };

  test("", "", 0, -1, true);
  test("", "", 0, 0, true);
  test("", "", 1, 0, false);
  test("", "a", 0, 1, false);
  test("", "a", 1, 1, false);
  test("abc", "a", 0, 3, true);
  test("abc", "a", 0, 2, true);
  test("abc", "a", 1, 3, false);
  test("abc", "a", 1, 2, false);
  test("abc", "c", 0, 3, false);
  test("abc", "c", 2, 3, true);
  test("abc", "c", 3, 3, false);
  test("abc", "c", 1, 2, false);
  test("abc", "b", 1, 2, true);
  test("abc", "abc", 0, -1, false);
  test("abc", "ab", 0, -1, true);
  test("abc", "abc", 0, 2, false);
  test("abc", "abc", 0, 3, true);
  test("abc", "abc", 0, 4, true);
  test("abc", "abc", 1, 3, false);
  test("abc", "abc", -1, 4, false);
  test("abc", "abc", -2, 10, false);
  test("abc", "", 0, 3, true);
  test("abc", "", 2, 3, true);
  test("abc", "", 2, 1, false);
  test("abc", "", 3, 3, true);
  test("abc", "", 4, 3, false);
  test("abc", "a", 0, 3, true);
  test("abc", "a", 0, 1, true);
  test("abc", "a", 3, 3, false);
  test("abc", "aabc", -1, 2, false);
  test("abc", "aabc", 0, 4, false);
  test("abc", "aabc", 3, 3, false);
}

TEST(StarlarkString, StartswithEndNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithEndNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.none_value());
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: startswith expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, StartswithWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.startswith() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, FindNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: find expected at least 1 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, FindTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: find() argument 1 must be string, not tuple");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, FindOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("find", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::int_t);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("banana", "", "0");
  test("banana", "an", "1");
  test("banana", "ban", "0");
  test("banana", "bb", "-1");
}

TEST(StarlarkString, FindStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, FindTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = str.dot("find", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::int_t);
    EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start;
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  /*
  for a in range(-5, 6):
      print('  test("abab", "ab", {}, {});'.format(a, "abab".find("ab", a)))
  */
  test("abab", "ab", -5, 0);
  test("abab", "ab", -4, 0);
  test("abab", "ab", -3, 2);
  test("abab", "ab", -2, 2);
  test("abab", "ab", -1, -1);
  test("abab", "ab", 0, 0);
  test("abab", "ab", 1, 2);
  test("abab", "ab", 2, 2);
  test("abab", "ab", 3, -1);
  test("abab", "ab", 4, -1);
  test("abab", "ab", 5, -1);
}

TEST(StarlarkString, FindThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = str.dot("find", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::int_t);
    EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start << ", end: " << end;
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  /*
  for a in range(-1, 2):
      for b in range(-1, 2):
          print('  test("", "", {}, {}, {});'.format(a, b, "".find("", a, b))
  */
  test("", "", -1, -1, 0);
  test("", "", -1, 0, 0);
  test("", "", -1, 1, 0);
  test("", "", 0, -1, 0);
  test("", "", 0, 0, 0);
  test("", "", 0, 1, 0);
  test("", "", 1, -1, -1);
  test("", "", 1, 0, -1);
  test("", "", 1, 1, -1);

  /*
  for a in range(-5, 6):
      for b in range(-5, 6):
          print('  test("abab", "ab", {}, {}, {});'.format(a, b, "abab".find("ab", a, b)))
  */
  test("abab", "ab", -5, -5, -1);
  test("abab", "ab", -5, -4, -1);
  test("abab", "ab", -5, -3, -1);
  test("abab", "ab", -5, -2, 0);
  test("abab", "ab", -5, -1, 0);
  test("abab", "ab", -5, 0, -1);
  test("abab", "ab", -5, 1, -1);
  test("abab", "ab", -5, 2, 0);
  test("abab", "ab", -5, 3, 0);
  test("abab", "ab", -5, 4, 0);
  test("abab", "ab", -5, 5, 0);
  test("abab", "ab", -4, -5, -1);
  test("abab", "ab", -4, -4, -1);
  test("abab", "ab", -4, -3, -1);
  test("abab", "ab", -4, -2, 0);
  test("abab", "ab", -4, -1, 0);
  test("abab", "ab", -4, 0, -1);
  test("abab", "ab", -4, 1, -1);
  test("abab", "ab", -4, 2, 0);
  test("abab", "ab", -4, 3, 0);
  test("abab", "ab", -4, 4, 0);
  test("abab", "ab", -4, 5, 0);
  test("abab", "ab", -3, -5, -1);
  test("abab", "ab", -3, -4, -1);
  test("abab", "ab", -3, -3, -1);
  test("abab", "ab", -3, -2, -1);
  test("abab", "ab", -3, -1, -1);
  test("abab", "ab", -3, 0, -1);
  test("abab", "ab", -3, 1, -1);
  test("abab", "ab", -3, 2, -1);
  test("abab", "ab", -3, 3, -1);
  test("abab", "ab", -3, 4, 2);
  test("abab", "ab", -3, 5, 2);
  test("abab", "ab", -2, -5, -1);
  test("abab", "ab", -2, -4, -1);
  test("abab", "ab", -2, -3, -1);
  test("abab", "ab", -2, -2, -1);
  test("abab", "ab", -2, -1, -1);
  test("abab", "ab", -2, 0, -1);
  test("abab", "ab", -2, 1, -1);
  test("abab", "ab", -2, 2, -1);
  test("abab", "ab", -2, 3, -1);
  test("abab", "ab", -2, 4, 2);
  test("abab", "ab", -2, 5, 2);
  test("abab", "ab", -1, -5, -1);
  test("abab", "ab", -1, -4, -1);
  test("abab", "ab", -1, -3, -1);
  test("abab", "ab", -1, -2, -1);
  test("abab", "ab", -1, -1, -1);
  test("abab", "ab", -1, 0, -1);
  test("abab", "ab", -1, 1, -1);
  test("abab", "ab", -1, 2, -1);
  test("abab", "ab", -1, 3, -1);
  test("abab", "ab", -1, 4, -1);
  test("abab", "ab", -1, 5, -1);
  test("abab", "ab", 0, -5, -1);
  test("abab", "ab", 0, -4, -1);
  test("abab", "ab", 0, -3, -1);
  test("abab", "ab", 0, -2, 0);
  test("abab", "ab", 0, -1, 0);
  test("abab", "ab", 0, 0, -1);
  test("abab", "ab", 0, 1, -1);
  test("abab", "ab", 0, 2, 0);
  test("abab", "ab", 0, 3, 0);
  test("abab", "ab", 0, 4, 0);
  test("abab", "ab", 0, 5, 0);
  test("abab", "ab", 1, -5, -1);
  test("abab", "ab", 1, -4, -1);
  test("abab", "ab", 1, -3, -1);
  test("abab", "ab", 1, -2, -1);
  test("abab", "ab", 1, -1, -1);
  test("abab", "ab", 1, 0, -1);
  test("abab", "ab", 1, 1, -1);
  test("abab", "ab", 1, 2, -1);
  test("abab", "ab", 1, 3, -1);
  test("abab", "ab", 1, 4, 2);
  test("abab", "ab", 1, 5, 2);
  test("abab", "ab", 2, -5, -1);
  test("abab", "ab", 2, -4, -1);
  test("abab", "ab", 2, -3, -1);
  test("abab", "ab", 2, -2, -1);
  test("abab", "ab", 2, -1, -1);
  test("abab", "ab", 2, 0, -1);
  test("abab", "ab", 2, 1, -1);
  test("abab", "ab", 2, 2, -1);
  test("abab", "ab", 2, 3, -1);
  test("abab", "ab", 2, 4, 2);
  test("abab", "ab", 2, 5, 2);
  test("abab", "ab", 3, -5, -1);
  test("abab", "ab", 3, -4, -1);
  test("abab", "ab", 3, -3, -1);
  test("abab", "ab", 3, -2, -1);
  test("abab", "ab", 3, -1, -1);
  test("abab", "ab", 3, 0, -1);
  test("abab", "ab", 3, 1, -1);
  test("abab", "ab", 3, 2, -1);
  test("abab", "ab", 3, 3, -1);
  test("abab", "ab", 3, 4, -1);
  test("abab", "ab", 3, 5, -1);
  test("abab", "ab", 4, -5, -1);
  test("abab", "ab", 4, -4, -1);
  test("abab", "ab", 4, -3, -1);
  test("abab", "ab", 4, -2, -1);
  test("abab", "ab", 4, -1, -1);
  test("abab", "ab", 4, 0, -1);
  test("abab", "ab", 4, 1, -1);
  test("abab", "ab", 4, 2, -1);
  test("abab", "ab", 4, 3, -1);
  test("abab", "ab", 4, 4, -1);
  test("abab", "ab", 4, 5, -1);
  test("abab", "ab", 5, -5, -1);
  test("abab", "ab", 5, -4, -1);
  test("abab", "ab", 5, -3, -1);
  test("abab", "ab", 5, -2, -1);
  test("abab", "ab", 5, -1, -1);
  test("abab", "ab", 5, 0, -1);
  test("abab", "ab", 5, 1, -1);
  test("abab", "ab", 5, 2, -1);
  test("abab", "ab", 5, 3, -1);
  test("abab", "ab", 5, 4, -1);
  test("abab", "ab", 5, 5, -1);
}

TEST(StarlarkString, FindFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: find expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, FindWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.find() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IndexNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: index expected at least 1 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IndexTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: index() argument 1 must be string, not tuple");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, IndexOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("index", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    if (expected < 0) {
      EXPECT_EQ(nullptr, result);
      ASSERT_THAT(error_callback.messages, SizeIs(1));
      EXPECT_EQ(error_callback.messages[0], "ValueError: substring not found");
    } else {
      ASSERT_NE(nullptr, result);
      EXPECT_EQ(result->type(), starlark_types::int_t);
      EXPECT_EQ(result->as_int64(), expected);
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };

  test("banana", "", 0);
  test("banana", "an", 1);
  test("banana", "ban", 0);
  test("banana", "bb", -1);
}

TEST(StarlarkString, IndexStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, IndexTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = str.dot("index", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    if (expected < 0) {
      EXPECT_EQ(nullptr, result);
      ASSERT_THAT(error_callback.messages, SizeIs(1));
      EXPECT_EQ(error_callback.messages[0], "ValueError: substring not found");
    } else {
      ASSERT_NE(nullptr, result);
      EXPECT_EQ(result->type(), starlark_types::int_t);
      EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start;
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };

  /*
  for a in range(-5, 6):
      print('  test("abab", "ab", {}, {});'.format(a, "abab".find("ab", a)))
  */
  test("abab", "ab", -5, 0);
  test("abab", "ab", -4, 0);
  test("abab", "ab", -3, 2);
  test("abab", "ab", -2, 2);
  test("abab", "ab", -1, -1);
  test("abab", "ab", 0, 0);
  test("abab", "ab", 1, 2);
  test("abab", "ab", 2, 2);
  test("abab", "ab", 3, -1);
  test("abab", "ab", 4, -1);
  test("abab", "ab", 5, -1);
}

TEST(StarlarkString, IndexThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = str.dot("index", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    if (expected < 0) {
      EXPECT_EQ(nullptr, result);
      ASSERT_THAT(error_callback.messages, SizeIs(1));
      EXPECT_EQ(error_callback.messages[0], "ValueError: substring not found");
    } else {
      ASSERT_NE(nullptr, result);
      EXPECT_EQ(result->type(), starlark_types::int_t);
      EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start << ", end: " << end;
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };

  /*
  for a in range(-1, 2):
      for b in range(-1, 2):
          print('  test("", "", {}, {}, {});'.format(a, b, "".find("", a, b))
  */
  test("", "", -1, -1, 0);
  test("", "", -1, 0, 0);
  test("", "", -1, 1, 0);
  test("", "", 0, -1, 0);
  test("", "", 0, 0, 0);
  test("", "", 0, 1, 0);
  test("", "", 1, -1, -1);
  test("", "", 1, 0, -1);
  test("", "", 1, 1, -1);

  /*
  for a in range(-5, 6):
      for b in range(-5, 6):
          print('  test("abab", "ab", {}, {}, {});'.format(a, b, "abab".find("ab", a, b)))
  */
  test("abab", "ab", -5, -5, -1);
  test("abab", "ab", -5, -4, -1);
  test("abab", "ab", -5, -3, -1);
  test("abab", "ab", -5, -2, 0);
  test("abab", "ab", -5, -1, 0);
  test("abab", "ab", -5, 0, -1);
  test("abab", "ab", -5, 1, -1);
  test("abab", "ab", -5, 2, 0);
  test("abab", "ab", -5, 3, 0);
  test("abab", "ab", -5, 4, 0);
  test("abab", "ab", -5, 5, 0);
  test("abab", "ab", -4, -5, -1);
  test("abab", "ab", -4, -4, -1);
  test("abab", "ab", -4, -3, -1);
  test("abab", "ab", -4, -2, 0);
  test("abab", "ab", -4, -1, 0);
  test("abab", "ab", -4, 0, -1);
  test("abab", "ab", -4, 1, -1);
  test("abab", "ab", -4, 2, 0);
  test("abab", "ab", -4, 3, 0);
  test("abab", "ab", -4, 4, 0);
  test("abab", "ab", -4, 5, 0);
  test("abab", "ab", -3, -5, -1);
  test("abab", "ab", -3, -4, -1);
  test("abab", "ab", -3, -3, -1);
  test("abab", "ab", -3, -2, -1);
  test("abab", "ab", -3, -1, -1);
  test("abab", "ab", -3, 0, -1);
  test("abab", "ab", -3, 1, -1);
  test("abab", "ab", -3, 2, -1);
  test("abab", "ab", -3, 3, -1);
  test("abab", "ab", -3, 4, 2);
  test("abab", "ab", -3, 5, 2);
  test("abab", "ab", -2, -5, -1);
  test("abab", "ab", -2, -4, -1);
  test("abab", "ab", -2, -3, -1);
  test("abab", "ab", -2, -2, -1);
  test("abab", "ab", -2, -1, -1);
  test("abab", "ab", -2, 0, -1);
  test("abab", "ab", -2, 1, -1);
  test("abab", "ab", -2, 2, -1);
  test("abab", "ab", -2, 3, -1);
  test("abab", "ab", -2, 4, 2);
  test("abab", "ab", -2, 5, 2);
  test("abab", "ab", -1, -5, -1);
  test("abab", "ab", -1, -4, -1);
  test("abab", "ab", -1, -3, -1);
  test("abab", "ab", -1, -2, -1);
  test("abab", "ab", -1, -1, -1);
  test("abab", "ab", -1, 0, -1);
  test("abab", "ab", -1, 1, -1);
  test("abab", "ab", -1, 2, -1);
  test("abab", "ab", -1, 3, -1);
  test("abab", "ab", -1, 4, -1);
  test("abab", "ab", -1, 5, -1);
  test("abab", "ab", 0, -5, -1);
  test("abab", "ab", 0, -4, -1);
  test("abab", "ab", 0, -3, -1);
  test("abab", "ab", 0, -2, 0);
  test("abab", "ab", 0, -1, 0);
  test("abab", "ab", 0, 0, -1);
  test("abab", "ab", 0, 1, -1);
  test("abab", "ab", 0, 2, 0);
  test("abab", "ab", 0, 3, 0);
  test("abab", "ab", 0, 4, 0);
  test("abab", "ab", 0, 5, 0);
  test("abab", "ab", 1, -5, -1);
  test("abab", "ab", 1, -4, -1);
  test("abab", "ab", 1, -3, -1);
  test("abab", "ab", 1, -2, -1);
  test("abab", "ab", 1, -1, -1);
  test("abab", "ab", 1, 0, -1);
  test("abab", "ab", 1, 1, -1);
  test("abab", "ab", 1, 2, -1);
  test("abab", "ab", 1, 3, -1);
  test("abab", "ab", 1, 4, 2);
  test("abab", "ab", 1, 5, 2);
  test("abab", "ab", 2, -5, -1);
  test("abab", "ab", 2, -4, -1);
  test("abab", "ab", 2, -3, -1);
  test("abab", "ab", 2, -2, -1);
  test("abab", "ab", 2, -1, -1);
  test("abab", "ab", 2, 0, -1);
  test("abab", "ab", 2, 1, -1);
  test("abab", "ab", 2, 2, -1);
  test("abab", "ab", 2, 3, -1);
  test("abab", "ab", 2, 4, 2);
  test("abab", "ab", 2, 5, 2);
  test("abab", "ab", 3, -5, -1);
  test("abab", "ab", 3, -4, -1);
  test("abab", "ab", 3, -3, -1);
  test("abab", "ab", 3, -2, -1);
  test("abab", "ab", 3, -1, -1);
  test("abab", "ab", 3, 0, -1);
  test("abab", "ab", 3, 1, -1);
  test("abab", "ab", 3, 2, -1);
  test("abab", "ab", 3, 3, -1);
  test("abab", "ab", 3, 4, -1);
  test("abab", "ab", 3, 5, -1);
  test("abab", "ab", 4, -5, -1);
  test("abab", "ab", 4, -4, -1);
  test("abab", "ab", 4, -3, -1);
  test("abab", "ab", 4, -2, -1);
  test("abab", "ab", 4, -1, -1);
  test("abab", "ab", 4, 0, -1);
  test("abab", "ab", 4, 1, -1);
  test("abab", "ab", 4, 2, -1);
  test("abab", "ab", 4, 3, -1);
  test("abab", "ab", 4, 4, -1);
  test("abab", "ab", 4, 5, -1);
  test("abab", "ab", 5, -5, -1);
  test("abab", "ab", 5, -4, -1);
  test("abab", "ab", 5, -3, -1);
  test("abab", "ab", 5, -2, -1);
  test("abab", "ab", 5, -1, -1);
  test("abab", "ab", 5, 0, -1);
  test("abab", "ab", 5, 1, -1);
  test("abab", "ab", 5, 2, -1);
  test("abab", "ab", 5, 3, -1);
  test("abab", "ab", 5, 4, -1);
  test("abab", "ab", 5, 5, -1);
}

TEST(StarlarkString, IndexFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: index expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IndexWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.index() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RfindNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rfind expected at least 1 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RfindTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rfind() argument 1 must be string, not tuple");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, RfindOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("rfind", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::int_t);
    EXPECT_EQ(result->as_int64(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("banana", "", 6);
  test("banana", "an", 3);
  test("banana", "ban", 0);
  test("banana", "bb", -1);
}

TEST(StarlarkString, RfindStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, RfindTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = str.dot("rfind", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::int_t);
    EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start;
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  /*
  for a in range(-5, 6):
      print('  test("abab", "ab", {}, {});'.format(a, "abab".rfind("ab", a)))
  */
  test("abab", "ab", -5, 2);
  test("abab", "ab", -4, 2);
  test("abab", "ab", -3, 2);
  test("abab", "ab", -2, 2);
  test("abab", "ab", -1, -1);
  test("abab", "ab", 0, 2);
  test("abab", "ab", 1, 2);
  test("abab", "ab", 2, 2);
  test("abab", "ab", 3, -1);
  test("abab", "ab", 4, -1);
  test("abab", "ab", 5, -1);
}

TEST(StarlarkString, RfindThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = str.dot("rfind", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::int_t);
    EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start << ", end: " << end;
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  /*
  for a in range(-1, 2):
      for b in range(-1, 2):
          print('  test("", "", {}, {}, {});'.format(a, b, "".rfind("", a, b))
  */
  test("", "", -1, -1, 0);
  test("", "", -1, 0, 0);
  test("", "", -1, 1, 0);
  test("", "", 0, -1, 0);
  test("", "", 0, 0, 0);
  test("", "", 0, 1, 0);
  test("", "", 1, -1, -1);
  test("", "", 1, 0, -1);
  test("", "", 1, 1, -1);

  /*
  for a in range(-5, 6):
      for b in range(-5, 6):
          print('  test("abab", "ab", {}, {}, {});'.format(a, b, "abab".rfind("ab", a, b)))
  */
  test("abab", "ab", -5, -5, -1);
  test("abab", "ab", -5, -4, -1);
  test("abab", "ab", -5, -3, -1);
  test("abab", "ab", -5, -2, 0);
  test("abab", "ab", -5, -1, 0);
  test("abab", "ab", -5, 0, -1);
  test("abab", "ab", -5, 1, -1);
  test("abab", "ab", -5, 2, 0);
  test("abab", "ab", -5, 3, 0);
  test("abab", "ab", -5, 4, 2);
  test("abab", "ab", -5, 5, 2);
  test("abab", "ab", -4, -5, -1);
  test("abab", "ab", -4, -4, -1);
  test("abab", "ab", -4, -3, -1);
  test("abab", "ab", -4, -2, 0);
  test("abab", "ab", -4, -1, 0);
  test("abab", "ab", -4, 0, -1);
  test("abab", "ab", -4, 1, -1);
  test("abab", "ab", -4, 2, 0);
  test("abab", "ab", -4, 3, 0);
  test("abab", "ab", -4, 4, 2);
  test("abab", "ab", -4, 5, 2);
  test("abab", "ab", -3, -5, -1);
  test("abab", "ab", -3, -4, -1);
  test("abab", "ab", -3, -3, -1);
  test("abab", "ab", -3, -2, -1);
  test("abab", "ab", -3, -1, -1);
  test("abab", "ab", -3, 0, -1);
  test("abab", "ab", -3, 1, -1);
  test("abab", "ab", -3, 2, -1);
  test("abab", "ab", -3, 3, -1);
  test("abab", "ab", -3, 4, 2);
  test("abab", "ab", -3, 5, 2);
  test("abab", "ab", -2, -5, -1);
  test("abab", "ab", -2, -4, -1);
  test("abab", "ab", -2, -3, -1);
  test("abab", "ab", -2, -2, -1);
  test("abab", "ab", -2, -1, -1);
  test("abab", "ab", -2, 0, -1);
  test("abab", "ab", -2, 1, -1);
  test("abab", "ab", -2, 2, -1);
  test("abab", "ab", -2, 3, -1);
  test("abab", "ab", -2, 4, 2);
  test("abab", "ab", -2, 5, 2);
  test("abab", "ab", -1, -5, -1);
  test("abab", "ab", -1, -4, -1);
  test("abab", "ab", -1, -3, -1);
  test("abab", "ab", -1, -2, -1);
  test("abab", "ab", -1, -1, -1);
  test("abab", "ab", -1, 0, -1);
  test("abab", "ab", -1, 1, -1);
  test("abab", "ab", -1, 2, -1);
  test("abab", "ab", -1, 3, -1);
  test("abab", "ab", -1, 4, -1);
  test("abab", "ab", -1, 5, -1);
  test("abab", "ab", 0, -5, -1);
  test("abab", "ab", 0, -4, -1);
  test("abab", "ab", 0, -3, -1);
  test("abab", "ab", 0, -2, 0);
  test("abab", "ab", 0, -1, 0);
  test("abab", "ab", 0, 0, -1);
  test("abab", "ab", 0, 1, -1);
  test("abab", "ab", 0, 2, 0);
  test("abab", "ab", 0, 3, 0);
  test("abab", "ab", 0, 4, 2);
  test("abab", "ab", 0, 5, 2);
  test("abab", "ab", 1, -5, -1);
  test("abab", "ab", 1, -4, -1);
  test("abab", "ab", 1, -3, -1);
  test("abab", "ab", 1, -2, -1);
  test("abab", "ab", 1, -1, -1);
  test("abab", "ab", 1, 0, -1);
  test("abab", "ab", 1, 1, -1);
  test("abab", "ab", 1, 2, -1);
  test("abab", "ab", 1, 3, -1);
  test("abab", "ab", 1, 4, 2);
  test("abab", "ab", 1, 5, 2);
  test("abab", "ab", 2, -5, -1);
  test("abab", "ab", 2, -4, -1);
  test("abab", "ab", 2, -3, -1);
  test("abab", "ab", 2, -2, -1);
  test("abab", "ab", 2, -1, -1);
  test("abab", "ab", 2, 0, -1);
  test("abab", "ab", 2, 1, -1);
  test("abab", "ab", 2, 2, -1);
  test("abab", "ab", 2, 3, -1);
  test("abab", "ab", 2, 4, 2);
  test("abab", "ab", 2, 5, 2);
  test("abab", "ab", 3, -5, -1);
  test("abab", "ab", 3, -4, -1);
  test("abab", "ab", 3, -3, -1);
  test("abab", "ab", 3, -2, -1);
  test("abab", "ab", 3, -1, -1);
  test("abab", "ab", 3, 0, -1);
  test("abab", "ab", 3, 1, -1);
  test("abab", "ab", 3, 2, -1);
  test("abab", "ab", 3, 3, -1);
  test("abab", "ab", 3, 4, -1);
  test("abab", "ab", 3, 5, -1);
  test("abab", "ab", 4, -5, -1);
  test("abab", "ab", 4, -4, -1);
  test("abab", "ab", 4, -3, -1);
  test("abab", "ab", 4, -2, -1);
  test("abab", "ab", 4, -1, -1);
  test("abab", "ab", 4, 0, -1);
  test("abab", "ab", 4, 1, -1);
  test("abab", "ab", 4, 2, -1);
  test("abab", "ab", 4, 3, -1);
  test("abab", "ab", 4, 4, -1);
  test("abab", "ab", 4, 5, -1);
  test("abab", "ab", 5, -5, -1);
  test("abab", "ab", 5, -4, -1);
  test("abab", "ab", 5, -3, -1);
  test("abab", "ab", 5, -2, -1);
  test("abab", "ab", 5, -1, -1);
  test("abab", "ab", 5, 0, -1);
  test("abab", "ab", 5, 1, -1);
  test("abab", "ab", 5, 2, -1);
  test("abab", "ab", 5, 3, -1);
  test("abab", "ab", 5, 4, -1);
  test("abab", "ab", 5, 5, -1);
}

TEST(StarlarkString, RfindFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rfind expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RfindWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.rfind() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RindexNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rindex expected at least 1 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RindexTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = str.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rindex() argument 1 must be string, not tuple");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, RindexOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("rindex", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    if (expected < 0) {
      EXPECT_EQ(nullptr, result);
      ASSERT_THAT(error_callback.messages, SizeIs(1));
      EXPECT_EQ(error_callback.messages[0], "ValueError: substring not found");
    } else {
      ASSERT_NE(nullptr, result);
      EXPECT_EQ(result->type(), starlark_types::int_t);
      EXPECT_EQ(result->as_int64(), expected);
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };

  test("banana", "", 6);
  test("banana", "an", 3);
  test("banana", "ban", 0);
  test("banana", "bb", -1);
}

TEST(StarlarkString, RindexStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("banana"sv);
  starlark_string param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(str.str(), "banana");
}

TEST(StarlarkString, RindexTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = str.dot("rindex", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    if (expected < 0) {
      EXPECT_EQ(nullptr, result);
      ASSERT_THAT(error_callback.messages, SizeIs(1));
      EXPECT_EQ(error_callback.messages[0], "ValueError: substring not found");
    } else {
      ASSERT_NE(nullptr, result);
      EXPECT_EQ(result->type(), starlark_types::int_t);
      EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start;
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };

  /*
  for a in range(-5, 6):
      print('  test("abab", "ab", {}, {});'.format(a, "abab".rfind("ab", a)))
  */
  test("abab", "ab", -5, 2);
  test("abab", "ab", -4, 2);
  test("abab", "ab", -3, 2);
  test("abab", "ab", -2, 2);
  test("abab", "ab", -1, -1);
  test("abab", "ab", 0, 2);
  test("abab", "ab", 1, 2);
  test("abab", "ab", 2, 2);
  test("abab", "ab", 3, -1);
  test("abab", "ab", 4, -1);
  test("abab", "ab", 5, -1);
}

TEST(StarlarkString, RindexThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = str.dot("rindex", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    if (expected < 0) {
      EXPECT_EQ(nullptr, result);
      ASSERT_THAT(error_callback.messages, SizeIs(1));
      EXPECT_EQ(error_callback.messages[0], "ValueError: substring not found");
    } else {
      ASSERT_NE(nullptr, result);
      EXPECT_EQ(result->type(), starlark_types::int_t);
      EXPECT_EQ(result->as_int64(), expected) << "Element: b'" << element << "', sub: b'" << sub << "', start: " << start << ", end: " << end;
      EXPECT_THAT(error_callback.messages, IsEmpty());
    }
  };

  /*
  for a in range(-1, 2):
      for b in range(-1, 2):
          print('  test("", "", {}, {}, {});'.format(a, b, "".rfind("", a, b))
  */
  test("", "", -1, -1, 0);
  test("", "", -1, 0, 0);
  test("", "", -1, 1, 0);
  test("", "", 0, -1, 0);
  test("", "", 0, 0, 0);
  test("", "", 0, 1, 0);
  test("", "", 1, -1, -1);
  test("", "", 1, 0, -1);
  test("", "", 1, 1, -1);

  /*
  for a in range(-5, 6):
      for b in range(-5, 6):
          print('  test("abab", "ab", {}, {}, {});'.format(a, b, "abab".rfind("ab", a, b)))
  */
  test("abab", "ab", -5, -5, -1);
  test("abab", "ab", -5, -4, -1);
  test("abab", "ab", -5, -3, -1);
  test("abab", "ab", -5, -2, 0);
  test("abab", "ab", -5, -1, 0);
  test("abab", "ab", -5, 0, -1);
  test("abab", "ab", -5, 1, -1);
  test("abab", "ab", -5, 2, 0);
  test("abab", "ab", -5, 3, 0);
  test("abab", "ab", -5, 4, 2);
  test("abab", "ab", -5, 5, 2);
  test("abab", "ab", -4, -5, -1);
  test("abab", "ab", -4, -4, -1);
  test("abab", "ab", -4, -3, -1);
  test("abab", "ab", -4, -2, 0);
  test("abab", "ab", -4, -1, 0);
  test("abab", "ab", -4, 0, -1);
  test("abab", "ab", -4, 1, -1);
  test("abab", "ab", -4, 2, 0);
  test("abab", "ab", -4, 3, 0);
  test("abab", "ab", -4, 4, 2);
  test("abab", "ab", -4, 5, 2);
  test("abab", "ab", -3, -5, -1);
  test("abab", "ab", -3, -4, -1);
  test("abab", "ab", -3, -3, -1);
  test("abab", "ab", -3, -2, -1);
  test("abab", "ab", -3, -1, -1);
  test("abab", "ab", -3, 0, -1);
  test("abab", "ab", -3, 1, -1);
  test("abab", "ab", -3, 2, -1);
  test("abab", "ab", -3, 3, -1);
  test("abab", "ab", -3, 4, 2);
  test("abab", "ab", -3, 5, 2);
  test("abab", "ab", -2, -5, -1);
  test("abab", "ab", -2, -4, -1);
  test("abab", "ab", -2, -3, -1);
  test("abab", "ab", -2, -2, -1);
  test("abab", "ab", -2, -1, -1);
  test("abab", "ab", -2, 0, -1);
  test("abab", "ab", -2, 1, -1);
  test("abab", "ab", -2, 2, -1);
  test("abab", "ab", -2, 3, -1);
  test("abab", "ab", -2, 4, 2);
  test("abab", "ab", -2, 5, 2);
  test("abab", "ab", -1, -5, -1);
  test("abab", "ab", -1, -4, -1);
  test("abab", "ab", -1, -3, -1);
  test("abab", "ab", -1, -2, -1);
  test("abab", "ab", -1, -1, -1);
  test("abab", "ab", -1, 0, -1);
  test("abab", "ab", -1, 1, -1);
  test("abab", "ab", -1, 2, -1);
  test("abab", "ab", -1, 3, -1);
  test("abab", "ab", -1, 4, -1);
  test("abab", "ab", -1, 5, -1);
  test("abab", "ab", 0, -5, -1);
  test("abab", "ab", 0, -4, -1);
  test("abab", "ab", 0, -3, -1);
  test("abab", "ab", 0, -2, 0);
  test("abab", "ab", 0, -1, 0);
  test("abab", "ab", 0, 0, -1);
  test("abab", "ab", 0, 1, -1);
  test("abab", "ab", 0, 2, 0);
  test("abab", "ab", 0, 3, 0);
  test("abab", "ab", 0, 4, 2);
  test("abab", "ab", 0, 5, 2);
  test("abab", "ab", 1, -5, -1);
  test("abab", "ab", 1, -4, -1);
  test("abab", "ab", 1, -3, -1);
  test("abab", "ab", 1, -2, -1);
  test("abab", "ab", 1, -1, -1);
  test("abab", "ab", 1, 0, -1);
  test("abab", "ab", 1, 1, -1);
  test("abab", "ab", 1, 2, -1);
  test("abab", "ab", 1, 3, -1);
  test("abab", "ab", 1, 4, 2);
  test("abab", "ab", 1, 5, 2);
  test("abab", "ab", 2, -5, -1);
  test("abab", "ab", 2, -4, -1);
  test("abab", "ab", 2, -3, -1);
  test("abab", "ab", 2, -2, -1);
  test("abab", "ab", 2, -1, -1);
  test("abab", "ab", 2, 0, -1);
  test("abab", "ab", 2, 1, -1);
  test("abab", "ab", 2, 2, -1);
  test("abab", "ab", 2, 3, -1);
  test("abab", "ab", 2, 4, 2);
  test("abab", "ab", 2, 5, 2);
  test("abab", "ab", 3, -5, -1);
  test("abab", "ab", 3, -4, -1);
  test("abab", "ab", 3, -3, -1);
  test("abab", "ab", 3, -2, -1);
  test("abab", "ab", 3, -1, -1);
  test("abab", "ab", 3, 0, -1);
  test("abab", "ab", 3, 1, -1);
  test("abab", "ab", 3, 2, -1);
  test("abab", "ab", 3, 3, -1);
  test("abab", "ab", 3, 4, -1);
  test("abab", "ab", 3, 5, -1);
  test("abab", "ab", 4, -5, -1);
  test("abab", "ab", 4, -4, -1);
  test("abab", "ab", 4, -3, -1);
  test("abab", "ab", 4, -2, -1);
  test("abab", "ab", 4, -1, -1);
  test("abab", "ab", 4, 0, -1);
  test("abab", "ab", 4, 1, -1);
  test("abab", "ab", 4, 2, -1);
  test("abab", "ab", 4, 3, -1);
  test("abab", "ab", 4, 4, -1);
  test("abab", "ab", 4, 5, -1);
  test("abab", "ab", 5, -5, -1);
  test("abab", "ab", 5, -4, -1);
  test("abab", "ab", 5, -3, -1);
  test("abab", "ab", 5, -2, -1);
  test("abab", "ab", 5, -1, -1);
  test("abab", "ab", 5, 0, -1);
  test("abab", "ab", 5, 1, -1);
  test("abab", "ab", 5, 2, -1);
  test("abab", "ab", 5, 3, -1);
  test("abab", "ab", 5, 4, -1);
  test("abab", "ab", 5, 5, -1);
}

TEST(StarlarkString, RindexFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rindex expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RindexWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.rindex() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: join() takes exactly one argument (0 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinString) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_string param1("banana"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'string' object is not iterable");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinEmptyTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_tuple param1(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::string_t);
  EXPECT_EQ(result->str(), "");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinTupleWithOneElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_string str2("xyz"sv);
  starlark_tuple param1(1);
  param1.add(&str2);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::string_t);
  EXPECT_EQ(result->str(), "xyz");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinTupleWithOneBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_tuple param1(1);
  param1.add(ctx.true_value());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: join() argument 1 must be string, not bool");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinTupleWithTwoElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_string str2("def"sv);
  starlark_string str3("ghi"sv);
  starlark_tuple param1(2);
  param1.add(&str2);
  param1.add(&str3);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::string_t);
  EXPECT_EQ(result->str(), "defabcghi");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  pos_args.push_back(&tuple);
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: join() takes exactly one argument (2 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, JoinWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = str.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: join() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, PartitionNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: partition() takes exactly one argument (0 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, PartitionString) {
  auto test = [](std::string_view element, std::string_view separator, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(separator);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("partition", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("abc", "banana", "(\"abc\", \"\", \"\")");
  test("abc", "a", "(\"\", \"a\", \"bc\")");
  test("abc", "b", "(\"a\", \"b\", \"c\")");
  test("abc", "c", "(\"ab\", \"c\", \"\")");
  test("aaa", "a", "(\"\", \"a\", \"aa\")");
}

TEST(StarlarkString, PartitionBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: partition() argument 1 must be string, not bool");
}

TEST(StarlarkString, PartitionEmptySeparator) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  auto* method = str.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkString, PartitionWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("x", ctx.zero());
  auto* method = str.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: partition() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RpartitionNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rpartition() takes exactly one argument (0 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RpartitionString) {
  auto test = [](std::string_view element, std::string_view separator, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(separator);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("rpartition", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("abc", "banana", "(\"\", \"\", \"abc\")");
  test("abc", "a", "(\"\", \"a\", \"bc\")");
  test("abc", "b", "(\"a\", \"b\", \"c\")");
  test("abc", "c", "(\"ab\", \"c\", \"\")");
  test("aaa", "a", "(\"aa\", \"a\", \"\")");
}

TEST(StarlarkString, RpartitionBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rpartition() argument 1 must be string, not bool");
}

TEST(StarlarkString, RpartitionEmptySeparator) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  auto* method = str.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkString, RpartitionWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("x", ctx.zero());
  auto* method = str.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rpartition() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, ReplaceWithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace expected at least 2 argument, got 0");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, ReplaceWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace expected at least 2 argument, got 1");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, ReplaceWithTwoArguments) {
  auto test = [](std::string_view element, std::string_view old, std::string_view new_, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(old);
    starlark_string param2(new_);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = str.dot("replace", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::string_t);
    EXPECT_EQ(result->as_string(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "", "", "");
  test("", "", "x", "x");
  test("a", "", "x", "xax");
  test("ab", "", "x", "xaxbx");

  test("", "y", "", "");
  test("", "y", "x", "");
  test("a", "y", "x", "a");
  test("ab", "y", "x", "ab");
  test("ayb", "y", "x", "axb");
  test("ayyb", "y", "x", "axxb");
  test("ayxyb", "y", "x", "axxxb");
  test("ayxyb", "y", "yy", "ayyxyyb");
}

TEST(StarlarkString, ReplaceWithTwoArgumentsOldAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace() argument 1 must be string, not bool");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, ReplaceWithTwoArgumentsNewAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace() argument 2 must be string, not bool");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, ReplaceWithThreeArguments) {
  auto test = [](std::string_view element, std::string_view old, std::string_view new_, int64_t count, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(old);
    starlark_string param2(new_);
    starlark_integer param3(count);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = str.dot("replace", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::string_t);
    EXPECT_EQ(result->as_string(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "", "", -1, "");
  test("", "", "", 0, "");
  test("", "", "", 1, "");
  test("", "", "x", -1, "x");
  test("", "", "x", 0, "");
  test("", "", "x", 1, "x");
  test("a", "", "x", -1, "xax");
  test("a", "", "x", 0, "a");
  test("a", "", "x", 1, "xa");
  test("ab", "", "x", -1, "xaxbx");
  test("ab", "", "x", 0, "ab");
  test("ab", "", "x", 1, "xab");

  test("", "y", "", -1, "");
  test("", "y", "", 0, "");
  test("", "y", "", 1, "");
  test("", "y", "x", -1, "");
  test("", "y", "x", 0, "");
  test("", "y", "x", 1, "");
  test("a", "y", "x", -1, "a");
  test("a", "y", "x", 0, "a");
  test("a", "y", "x", 1, "a");
  test("ab", "y", "x", -1, "ab");
  test("ab", "y", "x", 0, "ab");
  test("ab", "y", "x", 1, "ab");
  test("ayb", "y", "x", -1, "axb");
  test("ayb", "y", "x", 0, "ayb");
  test("ayb", "y", "x", 1, "axb");
  test("ayyb", "y", "x", -1, "axxb");
  test("ayyb", "y", "x", 0, "ayyb");
  test("ayyb", "y", "x", 1, "axyb");
  test("ayxyb", "y", "x", -1, "axxxb");
  test("ayxyb", "y", "x", 0, "ayxyb");
  test("ayxyb", "y", "x", 1, "axxyb");
  test("ayxyb", "y", "yy", -1, "ayyxyyb");
  test("ayxyb", "y", "yy", 0, "ayxyb");
  test("ayxyb", "y", "yy", 1, "ayyxyb");
}

TEST(StarlarkString, ReplaceWithThreeArgumentsCountAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  pos_args.push_back(ctx.empty_string());
  pos_args.push_back(ctx.true_value());
  auto* method = str.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'bool' object cannot be interpreted as an integer");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, ReplaceWithFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace expected at most 3 argument, got 4");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, ReplaceWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: string.replace() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsalnumNoArguments) {
  auto test = [](std::string_view element, bool expected_value) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;

    auto* method = str.dot("isalnum", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bool_t);
    EXPECT_EQ(result->truthy(), expected_value) << "'" << element << "'";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", false);
  test(" ", false);
  test("abc", true);
  test("123", true);
  test("abc123", true);
  test("abc123!@#", false);
  test("LettersOnly", true);
  test("Letters and spaces", false);
  test("µ", true);
  test("¼", true);
  test("\u3405", true);
}

TEST(StarlarkString, IsalphaNoArguments) {
  auto test = [](std::string_view element, bool expected_value) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;

    auto* method = str.dot("isalpha", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bool_t);
    EXPECT_EQ(result->truthy(), expected_value) << "'" << element << "'";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", false);
  test(" ", false);
  test("abc", true);
  test("123", false);
  test("abc123", false);
  test("abc123!@#", false);
  test("LettersOnly", true);
  test("Letters and spaces", false);
  test("µ", true);
  test("¼", false);
  test("\u3405", true);
}

TEST(StarlarkString, IsdigitNoArguments) {
  auto test = [](std::string_view element, bool expected_value) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;

    auto* method = str.dot("isdigit", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bool_t);
    EXPECT_EQ(result->truthy(), expected_value) << "'" << element << "'";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", false);
  test(" ", false);
  test("abc", false);
  test("123", true);
  test("abc123", false);
  test("abc123!@#", false);
  test("LettersOnly", false);
  test("Letters and spaces", false);
  test("µ", false);
  test("¼", false);
  test("\u3405", false);
}

TEST(StarlarkString, IsspaceNoArguments) {
  auto test = [](std::string_view element, bool expected_value) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;

    auto* method = str.dot("isspace", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bool_t);
    EXPECT_EQ(result->truthy(), expected_value) << "'" << element << "'";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", false);
  test(" ", true);
  test("abc", false);
  test("123", false);
  test("abc123", false);
  test("abc123!@#", false);
  test("LettersOnly", false);
  test("Letters and spaces", false);
  test("µ", false);
  test("¼", false);
  test("\u3405", false);
}

TEST(StarlarkString, IslowerNoArguments) {
  auto test = [](std::string_view element, bool expected_value) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;

    auto* method = str.dot("islower", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bool_t);
    EXPECT_EQ(result->truthy(), expected_value) << "'" << element << "'";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", false);
  test(" ", false);
  test("abc", true);
  test("Abc", false);
  test("ABC", false);
  test("123", false);
  test("abc123", true);
  test("Abc123", false);
  test("ABC123", false);
  test("abc123!@#", true);
  test("Abc123!@#", false);
  test("ABC123!@#", false);
  test("LettersOnly", false);
  test("Letters and spaces", false);
  test("µ", true);
  test("¼", false);
  test("\u3405", false);
}

TEST(StarlarkString, IsupperNoArguments) {
  auto test = [](std::string_view element, bool expected_value) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;

    auto* method = str.dot("isupper", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bool_t);
    EXPECT_EQ(result->truthy(), expected_value) << "'" << element << "'";
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", false);
  test(" ", false);
  test("abc", false);
  test("Abc", false);
  test("ABC", true);
  test("123", false);
  test("abc123", false);
  test("Abc123", false);
  test("ABC123", true);
  test("abc123!@#", false);
  test("Abc123!@#", false);
  test("ABC123!@#", true);
  test("LettersOnly", false);
  test("Letters and spaces", false);
  test("µ", false);
  test("¼", false);
  test("\u3405", false);
}

TEST(StarlarkString, IsalnumWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("isalnum", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isalnum() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsalphaWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("isalpha", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isalpha() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsdigitWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("isdigit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isdigit() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsspaceWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("isspace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isspace() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IslowerWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("islower", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: islower() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsupperWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = str.dot("isupper", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isupper() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsalnumWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("isalnum", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isalnum() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsalphaWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("isalpha", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isalpha() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsdigitWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("isdigit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isdigit() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsspaceWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("isspace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isspace() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IslowerWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("islower", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: islower() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, IsupperWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("isupper", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: isupper() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemovesuffixWithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("removesuffix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removesuffix() takes exactly one argument (0 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemovesuffixWithOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("removesuffix", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::string_t);
    EXPECT_EQ(result->as_string(), expected);

    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "", "");
  test("a", "", "a");
  test("", "a", "");
  test("aba", "a", "ab");
  test("aba", "b", "aba");
  test("aba", "x", "aba");
  test("aabaa", "aa", "aab");
}

TEST(StarlarkString, RemovesuffixWithOneArgumentBytes) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  auto* method = str.dot("removesuffix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removesuffix() argument 1 must be string, not bytes");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemovesuffixWithTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  pos_args.push_back(ctx.empty_string());
  auto* method = str.dot("removesuffix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removesuffix() takes exactly one argument (2 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemovesuffixWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("removesuffix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removesuffix() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemoveprefixWithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = str.dot("removeprefix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removeprefix() takes exactly one argument (0 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemoveprefixWithOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);
    starlark_string param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = str.dot("removeprefix", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::string_t);
    EXPECT_EQ(result->as_string(), expected);

    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "", "");
  test("a", "", "a");
  test("", "a", "");
  test("aba", "a", "ba");
  test("aba", "b", "aba");
  test("aba", "x", "aba");
  test("aabaa", "aa", "baa");
}

TEST(StarlarkString, RemoveprefixWithOneArgumentBytes) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  auto* method = str.dot("removeprefix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removeprefix() argument 1 must be string, not bytes");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemoveprefixWithTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  pos_args.push_back(ctx.empty_string());
  auto* method = str.dot("removeprefix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removeprefix() takes exactly one argument (2 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, RemoveprefixWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("removeprefix", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: removeprefix() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, UpperWithNoArguments) {
  auto test = [](std::string_view element, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    auto* method = str.dot("upper", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::string_t);
    EXPECT_EQ(result->as_string(), expected);

    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "");
  test("a", "A");
  test("A", "A");
  test("abc", "ABC");
  test("ABC", "ABC");
  test("abc1234", "ABC1234");
  test("ABC1234", "ABC1234");
  test("hello world!", "HELLO WORLD!");
  test("Hello World!", "HELLO WORLD!");
  test("\u0390", "\u0399\u0308\u0301");
  test("περιπτώσεις", "ΠΕΡΙΠΤΏΣΕΙΣ");
}

TEST(StarlarkString, LowerWithNoArguments) {
  auto test = [](std::string_view element, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_string str(element);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    auto* method = str.dot("lower", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    EXPECT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::string_t);
    EXPECT_EQ(result->as_string(), expected);

    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "");
  test("a", "a");
  test("A", "a");
  test("abc", "abc");
  test("ABC", "abc");
  test("abc1234", "abc1234");
  test("ABC1234", "abc1234");
  test("hello world!", "hello world!");
  test("Hello World!", "hello world!");
  test("\u0130", "\u0069\u0307");
  test("ΠΕΡΙΠΤΏΣΕΙΣ", "περιπτώσεις");
  test("Σ Σ", "σ σ");
  test(" Σ Σ ", " σ σ ");
  test("Σ.Σ.Σ", "σ.σ.ς");
  test(" Σ.Σ.Σ ", " σ.σ.ς ");
}

TEST(StarlarkString, UpperWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  auto* method = str.dot("upper", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: upper() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, TitleWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  auto* method = str.dot("title", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: title() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, LowerWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_string());
  auto* method = str.dot("lower", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: lower() takes no arguments (1 given)");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, UpperWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("upper", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: upper() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, TitleWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("title", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: title() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

TEST(StarlarkString, LowerWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_string str("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  named_args.insert("old", ctx.zero());
  auto* method = str.dot("lower", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: lower() takes no keyword arguments");
  EXPECT_EQ(str.str(), "abc");
}

}  // namespace
