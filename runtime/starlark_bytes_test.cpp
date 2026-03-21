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
  EXPECT_FALSE(starlark_bytes(""sv).equals(starlark_testing_function()));
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
    EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'string'");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes(""sv).binary_in(starlark_float(0), error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'float'");
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

TEST(StarlarkBytes, CountNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count expected at least 1 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, CountEmptyString) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_bytes empty(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&empty);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "4");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, Count) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountOverlap) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("aaaaaa"sv);
  starlark_bytes a_bytes("aa"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"aaaaaa\"");
}

TEST(StarlarkBytes, CountInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_integer a_int('a');

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_int);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountBigInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bigint a_int('a');

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_int);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountIntegerNegative) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.minus_one());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountIntegerTooBig) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_integer some_int(256);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&some_int);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountBigIntNegative) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bigint some_int(-1);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&some_int);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountBigIntTooBig) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bigint some_int(256);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&some_int);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.none_value());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'NoneType'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountFloat) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_float f_value('a');

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&f_value);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'float'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'tuple'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountTwoArgumentsStartInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("b"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "1");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountTwoArgumentsStartIntSkip) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("b"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.one());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountTwoArgumentsStartNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.none_value());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountTwoArgumentsStartBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.false_value());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountThreeArgumentsStartInt) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("a"sv);
  starlark_integer some_int(100);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(&some_int);
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountThreeArgumentsEndIntSkip) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.one());
  pos_args.push_back(ctx.minus_one());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "2");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountThreeArgumentsEndNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.none_value());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "3");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountThreeArgumentsEndBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes a_bytes("a"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&a_bytes);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.false_value());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, CountEmptyStringSameStartAndEnd) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_bytes empty(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&empty);
  pos_args.push_back(ctx.one());
  pos_args.push_back(ctx.one());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "1");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, CountEmptyStringEndBeforeStart) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_bytes empty(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&empty);
  pos_args.push_back(ctx.one());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::int_t);
  EXPECT_EQ(result->str(), "0");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, CountFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: count expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, CountWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("count", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.count() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: endswith expected at least 1 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithOneArgument) {
  auto test = [](std::string_view entry, std::string_view ending, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(entry);
    starlark_bytes param(ending);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param);
    auto* method = bytes.dot("endswith", ctx, error_callback);
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

TEST(StarlarkBytes, EndswithEmptyTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_FALSE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithTupleWithOneEntry) {
  auto test = [](std::string_view element, std::string_view entry, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_tuple tuple(1);
    starlark_bytes param(entry);
    tuple.add(&param);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&tuple);
    auto* method = bytes.dot("endswith", ctx, error_callback);
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

TEST(StarlarkBytes, EndswithString) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_string str(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'string'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithTupleWithString) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(1);
  starlark_string str(""sv);
  tuple.add(&str);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'string'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithTwoArguments) {
  auto test = [](std::string_view entry, std::string_view ending, int64_t start, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(entry);
    starlark_bytes param1(ending);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("endswith", ctx, error_callback);
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

TEST(StarlarkBytes, EndswithStartNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithStartNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.none_value());
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithThreeArguments) {
  auto test = [](std::string_view entry, std::string_view ending, int64_t start, int64_t end, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(entry);
    starlark_bytes param1(ending);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = bytes.dot("endswith", ctx, error_callback);
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
}

TEST(StarlarkBytes, EndswithEndNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithEndNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.none_value());
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: endswith expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, EndswithWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("endswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.endswith() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: startswith expected at least 1 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithOneArgument) {
  auto test = [](std::string_view entry, std::string_view beginning, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(entry);
    starlark_bytes param(beginning);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param);
    auto* method = bytes.dot("startswith", ctx, error_callback);
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

TEST(StarlarkBytes, StartswithEmptyTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_FALSE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithTupleWithOneEntry) {
  auto test = [](std::string_view element, std::string_view entry, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_tuple tuple(1);
    starlark_bytes param(entry);
    tuple.add(&param);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&tuple);
    auto* method = bytes.dot("startswith", ctx, error_callback);
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

TEST(StarlarkBytes, StartswithString) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_string str(""sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&str);
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'string'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithTupleWithString) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(1);
  starlark_string str(""sv);
  tuple.add(&str);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'string'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithTwoArguments) {
  auto test = [](std::string_view entry, std::string_view beginning, int64_t start, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(entry);
    starlark_bytes param1(beginning);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("startswith", ctx, error_callback);
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

TEST(StarlarkBytes, StartswithStartNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithStartNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.none_value());
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithThreeArguments) {
  auto test = [](std::string_view entry, std::string_view beginning, int64_t start, int64_t end, bool expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(entry);
    starlark_bytes param1(beginning);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = bytes.dot("startswith", ctx, error_callback);
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

TEST(StarlarkBytes, StartswithEndNotInteger) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithEndNone) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.none_value());
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_TRUE(result->truthy());

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: startswith expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StartswithWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("startswith", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.startswith() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, FindNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: find expected at least 1 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, FindTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'tuple'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, FindOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("find", ctx, error_callback);
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

TEST(StarlarkBytes, FindStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, FindTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("find", ctx, error_callback);
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

TEST(StarlarkBytes, FindThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = bytes.dot("find", ctx, error_callback);
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

TEST(StarlarkBytes, FindFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: find expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, FindWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("find", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.find() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, IndexNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: index expected at least 1 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, IndexTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'tuple'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, IndexOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("index", ctx, error_callback);
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

TEST(StarlarkBytes, IndexStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, IndexTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("index", ctx, error_callback);
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

TEST(StarlarkBytes, IndexThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = bytes.dot("index", ctx, error_callback);
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

TEST(StarlarkBytes, IndexFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: index expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, IndexWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("index", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.index() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RfindNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rfind expected at least 1 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RfindTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'tuple'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, RfindOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("rfind", ctx, error_callback);
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

TEST(StarlarkBytes, RfindStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, RfindTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("rfind", ctx, error_callback);
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

TEST(StarlarkBytes, RfindThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = bytes.dot("rfind", ctx, error_callback);
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

TEST(StarlarkBytes, RfindFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rfind expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RfindWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("rfind", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.rfind() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RindexNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rindex expected at least 1 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RindexTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'tuple'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, RindexOneArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("rindex", ctx, error_callback);
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

TEST(StarlarkBytes, RindexStartAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("banana"sv);
  starlark_bytes param1("an"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: slice indices must be integers, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"banana\"");
}

TEST(StarlarkBytes, RindexTwoArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start,  int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("rindex", ctx, error_callback);
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

TEST(StarlarkBytes, RindexThreeArgument) {
  auto test = [](std::string_view element, std::string_view sub, int64_t start, int64_t end, int64_t expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(sub);
    starlark_integer param2(start);
    starlark_integer param3(end);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = bytes.dot("rindex", ctx, error_callback);
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

TEST(StarlarkBytes, RindexFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rindex expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RindexWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("rindex", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.rindex() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: join() takes exactly one argument (0 given)");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinBytes) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_bytes param1("banana"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'bytes' object is not iterable");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinEmptyTuple) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple param1(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bytes_t);
  EXPECT_EQ(result->str(), "b\"\"");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinTupleWithOneElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple param1(1);
  param1.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bytes_t);
  EXPECT_EQ(result->str(), "b\"\\x01\"");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinTupleWithOneBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple param1(1);
  param1.add(ctx.true_value());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinTupleWithTwoElement) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple param1(2);
  param1.add(ctx.zero());
  param1.add(ctx.one());

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&param1);
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_NE(nullptr, result);
  EXPECT_EQ(result->type(), starlark_types::bytes_t);
  EXPECT_EQ(result->str(), "b\"\\x00abc\\x01\"");

  ASSERT_THAT(error_callback.messages, IsEmpty());
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);
  starlark_tuple tuple(0);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&tuple);
  pos_args.push_back(&tuple);
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: join() takes exactly one argument (2 given)");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, JoinWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("zero", ctx.zero());
  auto* method = bytes.dot("join", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: join() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StripNoArguments) {
  auto test = [](std::string_view element, std::string_view expected, std::string_view rexpected, std::string_view lexpected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    auto* method = bytes.dot("strip", ctx, error_callback);
    auto* rmethod = bytes.dot("rstrip", ctx, error_callback);
    auto* lmethod = bytes.dot("lstrip", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    ASSERT_NE(nullptr, rmethod);
    ASSERT_NE(nullptr, lmethod);

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    auto* rresult = rmethod->call(pos_args, named_args, ctx, error_callback);
    auto* lresult = lmethod->call(pos_args, named_args, ctx, error_callback);
    EXPECT_THAT(error_callback.messages, IsEmpty());
    ASSERT_NE(nullptr, result);
    ASSERT_NE(nullptr, rresult);
    ASSERT_NE(nullptr, rresult);
    EXPECT_EQ(result->type(), starlark_types::bytes_t);
    EXPECT_EQ(rresult->type(), starlark_types::bytes_t);
    EXPECT_EQ(lresult->type(), starlark_types::bytes_t);
    EXPECT_EQ(result->as_string(), expected);
    EXPECT_EQ(rresult->as_string(), rexpected);
    EXPECT_EQ(lresult->as_string(), lexpected);
  };

  test("", "", "", "");
  test(" ", "", "", "");
  test(" \t", "", "", "");
  test("  abc  ", "abc", "  abc", "abc  ");
  test("   abcdefghij  ", "abcdefghij", "   abcdefghij", "abcdefghij  ");
  test("  abcdefghij   ", "abcdefghij", "  abcdefghij", "abcdefghij   ");
  test("abcdefghij", "abcdefghij", "abcdefghij", "abcdefghij");
}

TEST(StarlarkBytes, StripOneArgument) {
  auto test = [](std::string_view element, std::string_view cutset, std::string_view expected, std::string_view rexpected, std::string_view lexpected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(cutset);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("strip", ctx, error_callback);
    auto* rmethod = bytes.dot("rstrip", ctx, error_callback);
    auto* lmethod = bytes.dot("lstrip", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    ASSERT_NE(nullptr, rmethod);
    ASSERT_NE(nullptr, lmethod);

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    auto* rresult = rmethod->call(pos_args, named_args, ctx, error_callback);
    auto* lresult = lmethod->call(pos_args, named_args, ctx, error_callback);
    EXPECT_THAT(error_callback.messages, IsEmpty());
    ASSERT_NE(nullptr, result);
    ASSERT_NE(nullptr, rresult);
    ASSERT_NE(nullptr, rresult);
    EXPECT_EQ(result->type(), starlark_types::bytes_t);
    EXPECT_EQ(rresult->type(), starlark_types::bytes_t);
    EXPECT_EQ(lresult->type(), starlark_types::bytes_t);
    EXPECT_EQ(result->as_string(), expected);
    EXPECT_EQ(rresult->as_string(), rexpected);
    EXPECT_EQ(lresult->as_string(), lexpected);
  };

  test("", "", "", "", "");
  test(" ", "", " ", " ", " ");
  test(" \t ", " ", "\t", " \t", "\t ");
  test("  abc  ", "x", "  abc  ", "  abc  ", "  abc  ");
  test("xxabcxx", "x", "abc", "xxabc", "abcxx");
  test("zyxabcdefghijzyx", "xyz", "abcdefghij", "zyxabcdefghij", "abcdefghijzyx");
  test("abcdefghij", "xyz", "abcdefghij", "abcdefghij", "abcdefghij");
  test("zzyyxx", "xyz", "", "", "");
}

TEST(StarlarkBytes, StripWithCutsetAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("strip", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RstripWithCutsetAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("rstrip", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, LstripWithCutsetAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("lstrip", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, StripWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("cutset", ctx.zero());
  auto* method = bytes.dot("strip", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: strip() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RstripWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("cutset", ctx.zero());
  auto* method = bytes.dot("rstrip", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rstrip() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, LstripWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("cutset", ctx.zero());
  auto* method = bytes.dot("lstrip", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: lstrip() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, PartitionNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: partition() takes exactly one argument (0 given)");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, PartitionBytes) {
  auto test = [](std::string_view element, std::string_view separator, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(separator);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("partition", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("abc", "banana", "(b\"abc\", b\"\", b\"\")");
  test("abc", "a", "(b\"\", b\"a\", b\"bc\")");
  test("abc", "b", "(b\"a\", b\"b\", b\"c\")");
  test("abc", "c", "(b\"ab\", b\"c\", b\"\")");
  test("aaa", "a", "(b\"\", b\"a\", b\"aa\")");
}

TEST(StarlarkBytes, PartitionBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
}

TEST(StarlarkBytes, PartitionEmptySeparator) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  auto* method = bytes.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkBytes, PartitionWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("x", ctx.zero());
  auto* method = bytes.dot("partition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: partition() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RpartitionNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rpartition() takes exactly one argument (0 given)");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RpartitionBytes) {
  auto test = [](std::string_view element, std::string_view separator, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(separator);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("rpartition", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    ASSERT_THAT(error_callback.messages, IsEmpty());
  };

  test("abc", "banana", "(b\"\", b\"\", b\"abc\")");
  test("abc", "a", "(b\"\", b\"a\", b\"bc\")");
  test("abc", "b", "(b\"a\", b\"b\", b\"c\")");
  test("abc", "c", "(b\"ab\", b\"c\", b\"\")");
  test("aaa", "a", "(b\"aa\", b\"a\", b\"\")");
}

TEST(StarlarkBytes, RpartitionBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
}

TEST(StarlarkBytes, RpartitionEmptySeparator) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  auto* method = bytes.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkBytes, RpartitionWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("x", ctx.zero());
  auto* method = bytes.dot("rpartition", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rpartition() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, SplitNoArguments) {
  auto test = [](std::string_view input, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    auto* method = bytes.dot("split", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "[]");
  test("  ", "[]");
  test("abc", "[b\"abc\"]");
  test("  abc  ", "[b\"abc\"]");
  test("  a  b  c  ", "[b\"a\", b\"b\", b\"c\"]");
}

TEST(StarlarkBytes, SplitNoneSeparatorArguments) {
  auto test = [](std::string_view input, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(ctx.none_value());
    auto* method = bytes.dot("split", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "[]");
  test("  ", "[]");
  test("abc", "[b\"abc\"]");
  test("  abc  ", "[b\"abc\"]");
  test("  a  b  c  ", "[b\"a\", b\"b\", b\"c\"]");
}

TEST(StarlarkBytes, SplitBoolSeparatorArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
}

TEST(StarlarkBytes, SplitEmptySeparatorArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkBytes, SplitOneArguments) {
  auto test = [](std::string_view input, std::string_view separator, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);
    starlark_bytes param1(separator);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("split", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "x", "[b\"\"]");
  test("  ", "x", "[b\"  \"]");
  test("axbxc", "x", "[b\"a\", b\"b\", b\"c\"]");
  test("xaxbxcx", "x", "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xxaxxbxxcxx", "x", "[b\"\", b\"\", b\"a\", b\"\", b\"b\", b\"\", b\"c\", b\"\", b\"\"]");
  test("xxaxxbxxcxx", "xx", "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xaxbxcx", "xx", "[b\"xaxbxcx\"]");
}

TEST(StarlarkBytes, SplitTwoArguments) {
  auto test = [](std::string_view input, std::string_view separator, int64_t maxsplit, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);
    starlark_bytes param1(separator);
    starlark_integer param2(maxsplit);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("split", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "x", -1, "[b\"\"]");
  test("", "x", 0, "[b\"\"]");
  test("", "x", 1, "[b\"\"]");
  test("  ", "x", -1, "[b\"  \"]");
  test("  ", "x", 0, "[b\"  \"]");
  test("  ", "x", 1, "[b\"  \"]");
  test("axbxc", "x", -1, "[b\"a\", b\"b\", b\"c\"]");
  test("axbxc", "x", 0, "[b\"axbxc\"]");
  test("axbxc", "x", 1, "[b\"a\", b\"bxc\"]");
  test("xaxbxcx", "x", -1, "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xaxbxcx", "x", 0, "[b\"xaxbxcx\"]");
  test("xaxbxcx", "x", 1, "[b\"\", b\"axbxcx\"]");
  test("xxaxxbxxcxx", "x", -1, "[b\"\", b\"\", b\"a\", b\"\", b\"b\", b\"\", b\"c\", b\"\", b\"\"]");
  test("xxaxxbxxcxx", "x", 0, "[b\"xxaxxbxxcxx\"]");
  test("xxaxxbxxcxx", "x", 1, "[b\"\", b\"xaxxbxxcxx\"]");
  test("xxaxxbxxcxx", "xx", -1, "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xxaxxbxxcxx", "xx", 0, "[b\"xxaxxbxxcxx\"]");
  test("xxaxxbxxcxx", "xx", 1, "[b\"\", b\"axxbxxcxx\"]");
  test("xaxbxcx", "xx", -1, "[b\"xaxbxcx\"]");
  test("xaxbxcx", "xx", 0, "[b\"xaxbxcx\"]");
  test("xaxbxcx", "xx", 1, "[b\"xaxbxcx\"]");
}

TEST(StarlarkBytes, SplitEmptySeparatorTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkBytes, SplitTwoArgumentsBoolSeparator) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
}

TEST(StarlarkBytes, SplitTwoArgumentsBoolMaxsplit) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'bool' object cannot be interpreted as an integer");
}

TEST(StarlarkBytes, SplitTwoArgumentsNoneSeparatorArguments) {
  auto test = [](std::string_view input, int64_t maxsplit, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);
    starlark_integer param2(maxsplit);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(ctx.none_value());
    pos_args.push_back(&param2);
    auto* method = bytes.dot("split", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", -1, "[]");
  test("", 0, "[]");
  test("", 1, "[]");
  test("  ", -1, "[]");
  test("  ", 0, "[]");
  test("  ", 1, "[]");
  test("abc", -1, "[b\"abc\"]");
  test("abc", 0, "[b\"abc\"]");
  test("abc", 1, "[b\"abc\"]");
  test("  abc  ", -1, "[b\"abc\"]");
  test("  abc  ", 0, "[b\"abc  \"]");
  test("  abc  ", 1, "[b\"abc\"]");
  test("  a  b  c  ", -1, "[b\"a\", b\"b\", b\"c\"]");
  test("  a  b  c  ", 0, "[b\"a  b  c  \"]");
  test("  a  b  c  ", 1, "[b\"a\", b\"b  c  \"]");
}

TEST(StarlarkBytes, SplitTwoArgumentsNoneSeparatorBoolMaxsplit) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'bool' object cannot be interpreted as an integer");
}

TEST(StarlarkBytes, SplitWithThreeArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: split expected at most 2 argument, got 3");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, SplitWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("maxsplit", ctx.zero());
  auto* method = bytes.dot("split", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.split() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RsplitNoArguments) {
  auto test = [](std::string_view input, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    auto* method = bytes.dot("rsplit", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "[]");
  test("  ", "[]");
  test("abc", "[b\"abc\"]");
  test("  abc  ", "[b\"abc\"]");
  test("  a  b  c  ", "[b\"a\", b\"b\", b\"c\"]");
}

TEST(StarlarkBytes, RsplitNoneSeparatorArguments) {
  auto test = [](std::string_view input, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(ctx.none_value());
    auto* method = bytes.dot("rsplit", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "[]");
  test("  ", "[]");
  test("abc", "[b\"abc\"]");
  test("  abc  ", "[b\"abc\"]");
  test("  a  b  c  ", "[b\"a\", b\"b\", b\"c\"]");
}

TEST(StarlarkBytes, RsplitBoolSeparatorArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
}

TEST(StarlarkBytes, RsplitEmptySeparatorArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkBytes, RsplitOneArguments) {
  auto test = [](std::string_view input, std::string_view separator, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);
    starlark_bytes param1(separator);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    auto* method = bytes.dot("rsplit", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "x", "[b\"\"]");
  test("  ", "x", "[b\"  \"]");
  test("axbxc", "x", "[b\"a\", b\"b\", b\"c\"]");
  test("xaxbxcx", "x", "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xxaxxbxxcxx", "x", "[b\"\", b\"\", b\"a\", b\"\", b\"b\", b\"\", b\"c\", b\"\", b\"\"]");
  test("xxaxxbxxcxx", "xx", "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xaxbxcx", "xx", "[b\"xaxbxcx\"]");
}

TEST(StarlarkBytes, RsplitTwoArguments) {
  auto test = [](std::string_view input, std::string_view separator, int64_t maxsplit, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);
    starlark_bytes param1(separator);
    starlark_integer param2(maxsplit);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("rsplit", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", "x", -1, "[b\"\"]");
  test("", "x", 0, "[b\"\"]");
  test("", "x", 1, "[b\"\"]");
  test("  ", "x", -1, "[b\"  \"]");
  test("  ", "x", 0, "[b\"  \"]");
  test("  ", "x", 1, "[b\"  \"]");
  test("axbxc", "x", -1, "[b\"a\", b\"b\", b\"c\"]");
  test("axbxc", "x", 0, "[b\"axbxc\"]");
  test("axbxc", "x", 1, "[b\"axb\", b\"c\"]");
  test("xaxbxcx", "x", -1, "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xaxbxcx", "x", 0, "[b\"xaxbxcx\"]");
  test("xaxbxcx", "x", 1, "[b\"xaxbxc\", b\"\"]");
  test("xxaxxbxxcxx", "x", -1, "[b\"\", b\"\", b\"a\", b\"\", b\"b\", b\"\", b\"c\", b\"\", b\"\"]");
  test("xxaxxbxxcxx", "x", 0, "[b\"xxaxxbxxcxx\"]");
  test("xxaxxbxxcxx", "x", 1, "[b\"xxaxxbxxcx\", b\"\"]");
  test("xxaxxbxxcxx", "xx", -1, "[b\"\", b\"a\", b\"b\", b\"c\", b\"\"]");
  test("xxaxxbxxcxx", "xx", 0, "[b\"xxaxxbxxcxx\"]");
  test("xxaxxbxxcxx", "xx", 1, "[b\"xxaxxbxxc\", b\"\"]");
  test("xaxbxcx", "xx", -1, "[b\"xaxbxcx\"]");
  test("xaxbxcx", "xx", 0, "[b\"xaxbxcx\"]");
  test("xaxbxcx", "xx", 1, "[b\"xaxbxcx\"]");
}

TEST(StarlarkBytes, RsplitEmptySeparatorTwoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.empty_bytes());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "ValueError: empty separator");
}

TEST(StarlarkBytes, RsplitTwoArgumentsBoolSeparator) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
}

TEST(StarlarkBytes, RsplitTwoArgumentsBoolMaxsplit) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(&bytes);
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'bool' object cannot be interpreted as an integer");
}

TEST(StarlarkBytes, RsplitTwoArgumentsNoneSeparatorArguments) {
  auto test = [](std::string_view input, int64_t maxsplit, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(input);
    starlark_integer param2(maxsplit);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(ctx.none_value());
    pos_args.push_back(&param2);
    auto* method = bytes.dot("rsplit", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->str(), expected);
    EXPECT_THAT(error_callback.messages, IsEmpty());
  };

  test("", -1, "[]");
  test("", 0, "[]");
  test("", 1, "[]");
  test("  ", -1, "[]");
  test("  ", 0, "[]");
  test("  ", 1, "[]");
  test("abc", -1, "[b\"abc\"]");
  test("abc", 0, "[b\"abc\"]");
  test("abc", 1, "[b\"abc\"]");
  test("  abc  ", -1, "[b\"abc\"]");
  test("  abc  ", 0, "[b\"  abc\"]");
  test("  abc  ", 1, "[b\"abc\"]");
  test("  a  b  c  ", -1, "[b\"a\", b\"b\", b\"c\"]");
  test("  a  b  c  ", 0, "[b\"  a  b  c\"]");
  test("  a  b  c  ", 1, "[b\"  a  b\", b\"c\"]");
}

TEST(StarlarkBytes, RsplitTwoArgumentsNoneSeparatorBoolMaxsplit) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.none_value());
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  ASSERT_EQ(nullptr, result);
  EXPECT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'bool' object cannot be interpreted as an integer");
}

TEST(StarlarkBytes, RsplitWithThreeArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: rsplit expected at most 2 argument, got 3");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, RsplitWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("maxsplit", ctx.zero());
  auto* method = bytes.dot("rsplit", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.rsplit() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, ReplaceWithNoArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  auto* method = bytes.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace expected at least 2 argument, got 0");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, ReplaceWithOneArgument) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace expected at least 2 argument, got 1");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, ReplaceWithTwoArguments) {
  auto test = [](std::string_view element, std::string_view old, std::string_view new_, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(old);
    starlark_bytes param2(new_);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    auto* method = bytes.dot("replace", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bytes_t);
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

TEST(StarlarkBytes, ReplaceWithTwoArgumentsOldAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.true_value());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, ReplaceWithTwoArgumentsNewAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: argument should be integer or bytes-like object, not 'bool'");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, ReplaceWithThreeArguments) {
  auto test = [](std::string_view element, std::string_view old, std::string_view new_, int64_t count, std::string_view expected) {
    error_handler error_callback;
    Arena arena;
    context ctx(arena);
    starlark_bytes bytes(element);
    starlark_bytes param1(old);
    starlark_bytes param2(new_);
    starlark_integer param3(count);

    starlark_obj::pos_args_t pos_args;
    starlark_obj::named_args_t named_args;
    pos_args.push_back(&param1);
    pos_args.push_back(&param2);
    pos_args.push_back(&param3);
    auto* method = bytes.dot("replace", ctx, error_callback);
    ASSERT_NE(nullptr, method);
    EXPECT_THAT(error_callback.messages, IsEmpty());

    auto* result = method->call(pos_args, named_args, ctx, error_callback);
    ASSERT_NE(nullptr, result);
    EXPECT_EQ(result->type(), starlark_types::bytes_t);
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

TEST(StarlarkBytes, ReplaceWithThreeArgumentsCountAsBool) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.true_value());
  auto* method = bytes.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: 'bool' object cannot be interpreted as an integer");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, ReplaceWithFourArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  pos_args.push_back(ctx.zero());
  auto* method = bytes.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: replace expected at most 3 argument, got 4");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

TEST(StarlarkBytes, ReplaceWithNamedArguments) {
  error_handler error_callback;
  Arena arena;
  context ctx(arena);
  starlark_bytes bytes("abc"sv);

  starlark_obj::pos_args_t pos_args;
  starlark_obj::named_args_t named_args;
  pos_args.push_back(ctx.zero());
  named_args.insert("old", ctx.zero());
  auto* method = bytes.dot("replace", ctx, error_callback);
  ASSERT_NE(nullptr, method);
  EXPECT_THAT(error_callback.messages, IsEmpty());

  auto* result = method->call(pos_args, named_args, ctx, error_callback);
  EXPECT_EQ(nullptr, result);

  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: bytes.replace() takes no keyword arguments");
  EXPECT_EQ(bytes.str(), "b\"abc\"");
}

}  // namespace
