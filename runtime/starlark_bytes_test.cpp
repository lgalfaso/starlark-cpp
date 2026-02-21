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
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

TEST(StarlarkBytes, Type) {
  EXPECT_EQ("bytes", starlark_bytes(""sv).type());
}

TEST(StarlarkBytes, Primitve) {
  EXPECT_TRUE(starlark_bytes(""sv).primitive());
}

TEST(StarlarkBytes, Str) {
  EXPECT_EQ("b\"'\"", starlark_bytes("'"sv).str());
  EXPECT_EQ("b\"\\\"\"", starlark_bytes("\""sv).str());
  EXPECT_EQ("b\"'\\\"\"", starlark_bytes("'\""sv).str());
  EXPECT_EQ("b\"\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\t\\n\\x0b\\x0c\\r\\x0e\\x0f"
            "\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1a\\x1b\\x1c\\x1d\\x1e\\x1f"
            " !\\\"#$%&\'()*+,-./0123456789:;<=>?"
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_"
            "`abcdefghijklmnopqrstuvwxyz{|}~\\x7f"
            "\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87\\x88\\x89\\x8a\\x8b\\x8c\\x8d\\x8e\\x8f"
            "\\x90\\x91\\x92\\x93\\x94\\x95\\x96\\x97\\x98\\x99\\x9a\\x9b\\x9c\\x9d\\x9e\\x9f"
            "\\xa0\\xa1\\xa2\\xa3\\xa4\\xa5\\xa6\\xa7\\xa8\\xa9\\xaa\\xab\\xac\\xad\\xae\\xaf"
            "\\xb0\\xb1\\xb2\\xb3\\xb4\\xb5\\xb6\\xb7\\xb8\\xb9\\xba\\xbb\\xbc\\xbd\\xbe\\xbf"
            "\\xc0\\xc1\\xc2\\xc3\\xc4\\xc5\\xc6\\xc7\\xc8\\xc9\\xca\\xcb\\xcc\\xcd\\xce\\xcf"
            "\\xd0\\xd1\\xd2\\xd3\\xd4\\xd5\\xd6\\xd7\\xd8\\xd9\\xda\\xdb\\xdc\\xdd\\xde\\xdf"
            "\\xe0\\xe1\\xe2\\xe3\\xe4\\xe5\\xe6\\xe7\\xe8\\xe9\\xea\\xeb\\xec\\xed\\xee\\xef"
            "\\xf0\\xf1\\xf2\\xf3\\xf4\\xf5\\xf6\\xf7\\xf8\\xf9\\xfa\\xfb\\xfc\\xfd\\xfe\\xff\"",
      starlark_bytes(std::string(
          "\000\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017"
          "\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036\037"
          "\040\041\042\043\044\045\046\047\050\051\052\053\054\055\056\057"
          "\060\061\062\063\064\065\066\067\070\071\072\073\074\075\076\077"
          "\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117"
          "\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137"
          "\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157"
          "\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177"
          "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
          "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
          "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"
          "\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
          "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"
          "\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
          "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357"
          "\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377", 256)).str());
}

TEST(StarlarkBytes, Truthy) {
  EXPECT_FALSE(starlark_bytes(""sv).truthy());
  EXPECT_TRUE(starlark_bytes("a"sv).truthy());
}

TEST(StarlarkBytes, Equals) {
  EXPECT_TRUE(starlark_bytes(""sv).equals(starlark_bytes(""sv)));
  EXPECT_TRUE(starlark_bytes("a"sv).equals(starlark_bytes("a"sv)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_bytes("a"sv)));
  // This is the NFKC decomposition.
  EXPECT_FALSE(starlark_bytes("\u03C9\u0301"sv).equals(starlark_bytes("\u03CE"sv)));

  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_bool(false)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_string(""sv)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_function()));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_list(0)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_none()));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_range(0, 1, 1)));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_set()));
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_tuple(0)));

  EXPECT_FALSE(starlark_bytes("0"sv).equals(starlark_integer(0)));
}

TEST(StarlarkBytes, Hash) {
  EXPECT_EQ(0, starlark_bytes(""sv).hash());
  EXPECT_EQ(-0x2d23137d629b3b16, starlark_bytes(std::string("\000", 1)).hash());
  EXPECT_EQ(0x7d34d047eac8d258, starlark_bytes("a"sv).hash());
  EXPECT_EQ(0x4fcdb76fe80ecc99, starlark_bytes("ab"sv).hash());
  EXPECT_EQ(-0x724cb4030369cd99, starlark_bytes("abc"sv).hash());
  EXPECT_EQ(-0x3867d33f599a68ac, starlark_bytes("abcd"sv).hash());
  EXPECT_EQ(-0x295e6451bc8f3946, starlark_bytes("abcde"sv).hash());
  EXPECT_EQ(0x405123fae156bb42, starlark_bytes("abcdef"sv).hash());
  EXPECT_EQ(0x2bbfd494ee0beadc, starlark_bytes("abcdefg"sv).hash());
  EXPECT_EQ(0x3d487434466b4028, starlark_bytes("abcdefgh"sv).hash());
  EXPECT_EQ(0x22745590a47f740d, starlark_bytes("abcdefghi"sv).hash());
  EXPECT_EQ(-0x6877fba8d314538c, starlark_bytes("abcdefghij"sv).hash());
  EXPECT_EQ(0x228401d4639024b6, starlark_bytes("abcdefghijk"sv).hash());
  EXPECT_EQ(0xd2b44ee272c9fb9, starlark_bytes("abcdefghijkl"sv).hash());
  EXPECT_EQ(-0x339dcaedb757cb43, starlark_bytes("abcdefghijklm"sv).hash());
  EXPECT_EQ(0x6a6885bf4173e958, starlark_bytes("abcdefghijklmn"sv).hash());
  EXPECT_EQ(-0x192f5a5c4ec6830a, starlark_bytes("abcdefghijklmno"sv).hash());
  EXPECT_EQ(-0x4468bef2850510a7, starlark_bytes("abcdefghijklmnop"sv).hash());
  EXPECT_EQ(0x357ae4009a9af80b, starlark_bytes("abcdefghijklmnopq"sv).hash());
  EXPECT_EQ(0x706bdfdacdc5ae41, starlark_bytes("abcdefghijklmnopqr"sv).hash());
  EXPECT_EQ(0x3f3328d017b33f9c, starlark_bytes("abcdefghijklmnopqrs"sv).hash());
  EXPECT_EQ(0x6d99ffc4406e044f, starlark_bytes("abcdefghijklmnopqrst"sv).hash());
  EXPECT_EQ(0x4d39b99a9b86a475, starlark_bytes("abcdefghijklmnopqrstu"sv).hash());
  EXPECT_EQ(0x579b4578d9828acd, starlark_bytes("abcdefghijklmnopqrstuv"sv).hash());
  EXPECT_EQ(0x79d441fd16593245, starlark_bytes("abcdefghijklmnopqrstuvw"sv).hash());
  EXPECT_EQ(0x4873bac806c24942, starlark_bytes("abcdefghijklmnopqrstuvwx"sv).hash());
  EXPECT_EQ(0x1c8b9cf72befb029, starlark_bytes("abcdefghijklmnopqrstuvwxy"sv).hash());
  EXPECT_EQ(0x35d9b8008d993f24, starlark_bytes("abcdefghijklmnopqrstuvwxyz"sv).hash());
  EXPECT_EQ(-0x5790e4047531b674, starlark_bytes("abcdefghijklmnopqrstuvwxyz0"sv).hash());
  EXPECT_EQ(-0xa5df0823b0bf31c, starlark_bytes("abcdefghijklmnopqrstuvwxyz01"sv).hash());
  EXPECT_EQ(0x6675f1d741c8c5af, starlark_bytes("abcdefghijklmnopqrstuvwxyz012"sv).hash());
  EXPECT_EQ(0x6c4187f34ae442f3, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123"sv).hash());
  EXPECT_EQ(-0x34291a48f6e60800, starlark_bytes("abcdefghijklmnopqrstuvwxyz01234"sv).hash());
  EXPECT_EQ(-0x46d86e557938d5a6, starlark_bytes("abcdefghijklmnopqrstuvwxyz012345"sv).hash());
  EXPECT_EQ(-0x79258352eaae9d9a, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456"sv).hash());
  EXPECT_EQ(0x71b2105b81a17454, starlark_bytes("abcdefghijklmnopqrstuvwxyz01234567"sv).hash());
  EXPECT_EQ(0x41848a1fc73d37d1, starlark_bytes("abcdefghijklmnopqrstuvwxyz012345678"sv).hash());
  EXPECT_EQ(0x1048b51015c84576, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456789"sv).hash());
  EXPECT_EQ(-0x23638df788af12ee, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456789@"sv).hash());
  EXPECT_EQ(0x5622cc50881d8b5b, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456789@!"sv).hash());
}

TEST(StarlarkBytes, Order) {
  error_handler error_callback;

  EXPECT_THAT(starlark_bytes(""sv).cmp(starlark_bytes(""sv), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_bytes(""sv).cmp(starlark_bytes("a"sv), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_bytes("a"sv).cmp(starlark_bytes("a"sv), "cmp", error_callback), Eq(0));
  EXPECT_THAT(starlark_bytes("a"sv).cmp(starlark_bytes(""sv), "cmp", error_callback), Gt(0));
  EXPECT_THAT(starlark_bytes("a"sv).cmp(starlark_bytes("b"sv), "cmp", error_callback), Lt(0));
  EXPECT_THAT(starlark_bytes("b"sv).cmp(starlark_bytes("a"sv), "cmp", error_callback), Gt(0));
}

TEST(StarlarkBytes, OrderErrors) {
  error_handler error_callback;
  EXPECT_FALSE(starlark_bytes(""sv).cmp(starlark_string(""sv), "<", error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'bytes' and 'string'");
}

TEST(StarlarkBytes, BinaryIn) {
  error_handler error_callback;
  EXPECT_TRUE(starlark_bytes(""sv).binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).binary_in(starlark_bytes(""sv), error_callback));
  EXPECT_FALSE(starlark_bytes("a"sv).binary_in(starlark_bytes("b"sv), error_callback));
  EXPECT_FALSE(starlark_bytes("a"sv).binary_in(starlark_integer('b'), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).binary_in(starlark_integer('a'), error_callback));
  EXPECT_TRUE(starlark_bytes("a"sv).binary_in(starlark_bigint('a'), error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, BinaryInErrors) {
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_string(""sv), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: a bytes-like object is required, not 'string'");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_float(0), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: a bytes-like object is required, not 'float'");
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

TEST(StarlarkBytes, BinaryPlus) {
  starlark_bytes bytes_1("abc"sv);
  starlark_bytes bytes_2("def"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = bytes_1.binary_plus(bytes_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "b\"abcdef\"");
}

TEST(StarlarkBytes, BinaryPlusNotList) {
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = bytes.binary_plus(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't concat tuple to bytes");
}

TEST(StarlarkBytes, PlusEqualsAssign) {
  starlark_bytes bytes_1("abc"sv);
  starlark_bytes bytes_2("def"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = bytes_1.plus_equals_assign(bytes_2, ctx, error_callback);

  ASSERT_NE(result, nullptr);
  EXPECT_EQ(result->str(), "b\"abcdef\"");
}

TEST(StarlarkBytes, PlusEqualsAssignNotList) {
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = bytes.plus_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't concat tuple to bytes");
}

TEST(StarlarkBytes, StarEqualsAssign) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_bytes bytes0(""sv);
  starlark_bytes bytes("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = bytes.star_equals_assign(two, ctx, error_callback);
  auto* result_2 = bytes.star_equals_assign(three, ctx, error_callback);
  auto* result_3 = bytes.star_equals_assign(minus_two, ctx, error_callback);
  auto* result_4 = bytes.star_equals_assign(minus_one, ctx, error_callback);
  auto* result_5 = bytes0.star_equals_assign(big, ctx, error_callback);
  auto* result_6 = bytes0.star_equals_assign(two, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "b\"abcabc\"");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "b\"abcabcabc\"");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "b\"\"");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "b\"\"");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "b\"\"");
  ASSERT_NE(result_6, nullptr);
  EXPECT_EQ(result_6->str(), "b\"\"");
}

TEST(StarlarkBytes, StarEqualsReverse) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_bytes bytes0(""sv);
  starlark_bytes bytes("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = two.star_equals_assign(bytes, ctx, error_callback);
  auto* result_2 = three.star_equals_assign(bytes, ctx, error_callback);
  auto* result_3 = minus_two.star_equals_assign(bytes, ctx, error_callback);
  auto* result_4 = minus_one.star_equals_assign(bytes, ctx, error_callback);
  auto* result_5 = big.star_equals_assign(bytes0, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "b\"abcabc\"");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "b\"abcabcabc\"");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "b\"\"");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "b\"\"");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "b\"\"");
}

TEST(StarlarkBytes, StarEqualsNotInt) {
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = bytes.star_equals_assign(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkBytes, BinaryStar) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_bytes bytes0(""sv);
  starlark_bytes bytes("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = bytes.binary_star(two, ctx, error_callback);
  auto* result_2 = bytes.binary_star(three, ctx, error_callback);
  auto* result_3 = bytes.binary_star(minus_two, ctx, error_callback);
  auto* result_4 = bytes.binary_star(minus_one, ctx, error_callback);
  auto* result_5 = bytes0.binary_star(big, ctx, error_callback);
  auto* result_6 = bytes0.binary_star(two, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "b\"abcabc\"");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "b\"abcabcabc\"");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "b\"\"");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "b\"\"");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "b\"\"");
  ASSERT_NE(result_6, nullptr);
  EXPECT_EQ(result_6->str(), "b\"\"");
}

TEST(StarlarkBytes, BinaryStarReverse) {
  starlark_bigint minus_two(-2);
  starlark_integer minus_one(-1);
  starlark_integer two(2);
  starlark_bigint three(3);
  starlark_bigint big(number::one() << 64);
  starlark_bytes bytes0(""sv);
  starlark_bytes bytes("abc"sv);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result_1 = two.binary_star(bytes, ctx, error_callback);
  auto* result_2 = three.binary_star(bytes, ctx, error_callback);
  auto* result_3 = minus_two.binary_star(bytes, ctx, error_callback);
  auto* result_4 = minus_one.binary_star(bytes, ctx, error_callback);
  auto* result_5 = big.binary_star(bytes0, ctx, error_callback);

  ASSERT_NE(result_1, nullptr);
  EXPECT_EQ(result_1->str(), "b\"abcabc\"");
  ASSERT_NE(result_2, nullptr);
  EXPECT_EQ(result_2->str(), "b\"abcabcabc\"");
  ASSERT_NE(result_3, nullptr);
  EXPECT_EQ(result_3->str(), "b\"\"");
  ASSERT_NE(result_4, nullptr);
  EXPECT_EQ(result_4->str(), "b\"\"");
  ASSERT_NE(result_5, nullptr);
  EXPECT_EQ(result_5->str(), "b\"\"");
}

TEST(StarlarkBytes, BinaryStarNotInt) {
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(0);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = bytes.binary_star(tuple, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: can't multiply sequence by non-int of type 'tuple'");
}

TEST(StarlarkBytes, BinaryStarTooBig) {
  starlark_bytes bytes("abc"sv);
  starlark_bigint big(number::one() << 64);
  Arena arena;
  context ctx(arena);
  error_handler error_callback;

  auto* result = bytes.binary_star(big, ctx, error_callback);
  EXPECT_EQ(result, nullptr);
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: sequences must be at most 2147483647 elements");
}

TEST(StarlarkBytes, Len) {
  error_handler error_callback;

  EXPECT_EQ(0, starlark_bytes(""sv).len(true, error_callback));
  EXPECT_EQ(3, starlark_bytes("abc"sv).len(true, error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, Index) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  EXPECT_EQ(bytes.index(starlark_integer(-3), ctx, error_callback)->repr(), "b\"a\"");
  EXPECT_EQ(bytes.index(starlark_integer(-2), ctx, error_callback)->repr(), "b\"b\"");
  EXPECT_EQ(bytes.index(starlark_integer(-1), ctx, error_callback)->repr(), "b\"c\"");
  EXPECT_EQ(bytes.index(starlark_integer(0), ctx, error_callback)->repr(), "b\"a\"");
  EXPECT_EQ(bytes.index(starlark_integer(1), ctx, error_callback)->repr(), "b\"b\"");
  EXPECT_EQ(bytes.index(starlark_integer(2), ctx, error_callback)->repr(), "b\"c\"");
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, IndexOutOfRange1) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  EXPECT_EQ(nullptr, bytes.index(starlark_integer(-4), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("IndexError: bytes index out of range", error_callback.messages[0]);
}

TEST(StarlarkBytes, IndexOutOfRange2) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  EXPECT_EQ(nullptr, bytes.index(starlark_integer(3), ctx, error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ("IndexError: bytes index out of range", error_callback.messages[0]);
}

TEST(StarlarkBytes, SliceRange) {
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
    starlark_bytes bytes0(""sv);
    starlark_bytes bytes1("a"sv);
    starlark_bytes bytes2("ab"sv);
    starlark_bytes bytes3("abc"sv);
    starlark_bytes bytes4("abcd"sv);
    starlark_bytes bytes5("abcde"sv);

    auto* result0 = bytes0.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result1 = bytes1.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result2 = bytes2.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result3 = bytes3.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result4 = bytes4.slice_range(*start, *end, *stride, ctx, error_callback);
    auto* result5 = bytes5.slice_range(*start, *end, *stride, ctx, error_callback);

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
      return 'test({}, {}, {}, "{}", "{}", "{}", "{}", "{}", "{}");'.format(tt(a), tt(b), tt(c), *[str(b'abcde'[:x][a:b:c]) for x in range(6)])
  "\n  ".join([rr(a,b,c) for a in (None, -1, 0, 1) for b in (None, -1, 0, 1) for c in (None, -1, 1)])
  ```
  */

  test(ctx.none_value(), ctx.none_value(), ctx.none_value(), "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"", "b\"abcde\"");
  test(ctx.none_value(), ctx.none_value(), ctx.minus_one(), "b\"\"", "b\"a\"", "b\"ba\"", "b\"cba\"", "b\"dcba\"", "b\"edcba\"");
  test(ctx.none_value(), ctx.none_value(), ctx.one(), "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"", "b\"abcde\"");
  test(ctx.none_value(), ctx.minus_one(), ctx.none_value(), "b\"\"", "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"");
  test(ctx.none_value(), ctx.minus_one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.none_value(), ctx.minus_one(), ctx.one(), "b\"\"", "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"");
  test(ctx.none_value(), ctx.zero(), ctx.none_value(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.none_value(), ctx.zero(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"b\"", "b\"cb\"", "b\"dcb\"", "b\"edcb\"");
  test(ctx.none_value(), ctx.zero(), ctx.one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.none_value(), ctx.one(), ctx.none_value(), "b\"\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"");
  test(ctx.none_value(), ctx.one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"c\"", "b\"dc\"", "b\"edc\"");
  test(ctx.none_value(), ctx.one(), ctx.one(), "b\"\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"");

  test(ctx.minus_one(), ctx.none_value(), ctx.none_value(), "b\"\"", "b\"a\"", "b\"b\"", "b\"c\"", "b\"d\"", "b\"e\"");
  test(ctx.minus_one(), ctx.none_value(), ctx.minus_one(), "b\"\"", "b\"a\"", "b\"ba\"", "b\"cba\"", "b\"dcba\"", "b\"edcba\"");
  test(ctx.minus_one(), ctx.none_value(), ctx.one(), "b\"\"", "b\"a\"", "b\"b\"", "b\"c\"", "b\"d\"", "b\"e\"");
  test(ctx.minus_one(), ctx.minus_one(), ctx.none_value(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.minus_one(), ctx.minus_one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.minus_one(), ctx.minus_one(), ctx.one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.minus_one(), ctx.zero(), ctx.none_value(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.minus_one(), ctx.zero(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"b\"", "b\"cb\"", "b\"dcb\"", "b\"edcb\"");
  test(ctx.minus_one(), ctx.zero(), ctx.one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.minus_one(), ctx.one(), ctx.none_value(), "b\"\"", "b\"a\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.minus_one(), ctx.one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"c\"", "b\"dc\"", "b\"edc\"");
  test(ctx.minus_one(), ctx.one(), ctx.one(), "b\"\"", "b\"a\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");

  test(ctx.zero(), ctx.none_value(), ctx.none_value(), "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"", "b\"abcde\"");
  test(ctx.zero(), ctx.none_value(), ctx.minus_one(), "b\"\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"");
  test(ctx.zero(), ctx.none_value(), ctx.one(), "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"", "b\"abcde\"");
  test(ctx.zero(), ctx.minus_one(), ctx.none_value(), "b\"\"", "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"");
  test(ctx.zero(), ctx.minus_one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.zero(), ctx.minus_one(), ctx.one(), "b\"\"", "b\"\"", "b\"a\"", "b\"ab\"", "b\"abc\"", "b\"abcd\"");
  test(ctx.zero(), ctx.zero(), ctx.none_value(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.zero(), ctx.zero(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.zero(), ctx.zero(), ctx.one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.zero(), ctx.one(), ctx.none_value(), "b\"\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"");
  test(ctx.zero(), ctx.one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.zero(), ctx.one(), ctx.one(), "b\"\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"", "b\"a\"");

  test(ctx.one(), ctx.none_value(), ctx.none_value(), "b\"\"", "b\"\"", "b\"b\"", "b\"bc\"", "b\"bcd\"", "b\"bcde\"");
  test(ctx.one(), ctx.none_value(), ctx.minus_one(), "b\"\"", "b\"a\"", "b\"ba\"", "b\"ba\"", "b\"ba\"", "b\"ba\"");
  test(ctx.one(), ctx.none_value(), ctx.one(), "b\"\"", "b\"\"", "b\"b\"", "b\"bc\"", "b\"bcd\"", "b\"bcde\"");
  test(ctx.one(), ctx.minus_one(), ctx.none_value(), "b\"\"", "b\"\"", "b\"\"", "b\"b\"", "b\"bc\"", "b\"bcd\"");
  test(ctx.one(), ctx.minus_one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.one(), ctx.minus_one(), ctx.one(), "b\"\"", "b\"\"", "b\"\"", "b\"b\"", "b\"bc\"", "b\"bcd\"");
  test(ctx.one(), ctx.zero(), ctx.none_value(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.one(), ctx.zero(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"b\"", "b\"b\"", "b\"b\"", "b\"b\"");
  test(ctx.one(), ctx.zero(), ctx.one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.one(), ctx.one(), ctx.none_value(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.one(), ctx.one(), ctx.minus_one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
  test(ctx.one(), ctx.one(), ctx.one(), "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"", "b\"\"");
}

TEST(StarlarkBytes, SliceRangeBoolStart) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_bytes bytes("abcdef"sv);

  auto* result = bytes.slice_range(*ctx.true_value(), *ctx.none_value(), *ctx.none_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("b\"abcdef\"", bytes.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkBytes, SliceRangeBoolEnd) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_bytes bytes("abcdef"sv);

  auto* result = bytes.slice_range(*ctx.none_value(), *ctx.false_value(), *ctx.none_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("b\"abcdef\"", bytes.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkBytes, SliceRangeBoolStride) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_bytes bytes("abcdef"sv);

  auto* result = bytes.slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.false_value(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("b\"abcdef\"", bytes.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
}

TEST(StarlarkBytes, SliceRangeZeroStride) {
  Arena arena;
  context ctx(arena);
  error_handler error_callback;
  starlark_bytes bytes("abcdef"sv);

  auto* result = bytes.slice_range(*ctx.none_value(), *ctx.none_value(), *ctx.zero(), ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_EQ("b\"abcdef\"", bytes.str());
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: slice step cannot be zero");
}

}  // namespace
