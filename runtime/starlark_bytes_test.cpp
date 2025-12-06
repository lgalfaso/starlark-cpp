// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include <string>
#include <vector>

#include "runtime/starlark_function.hpp"
#include "runtime/starlark_bigint.hpp"
#include "runtime/starlark_bool.hpp"
#include "runtime/starlark_bytes.hpp"
#include "runtime/starlark_dictionary.hpp"
#include "runtime/starlark_float.hpp"
#include "runtime/starlark_integer.hpp"
#include "runtime/starlark_list.hpp"
#include "runtime/starlark_none.hpp"
#include "runtime/starlark_range.hpp"
#include "runtime/starlark_set.hpp"
#include "runtime/starlark_string.hpp"
#include "runtime/starlark_struct.hpp"
#include "runtime/starlark_tuple.hpp"

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
using ::starlark::runtime::starlark_range;
using ::starlark::runtime::starlark_set;
using ::starlark::runtime::starlark_string;
using ::starlark::runtime::starlark_struct;
using ::starlark::runtime::starlark_tuple;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::Lt;
using ::testing::SizeIs;

namespace {

struct error_handler : public error_fn {
  void add_error(std::string_view error_msg) override {
    messages.push_back(std::string(error_msg));
  }

  std::vector<std::string> messages;
};

TEST(StarlarkBytes, Type) {
  EXPECT_EQ("bytes", starlark_bytes("").type());
}

TEST(StarlarkBytes, Str) {
  EXPECT_EQ("b\"'\"", starlark_bytes("'").str());
  EXPECT_EQ("b'\"'", starlark_bytes("\"").str());
  EXPECT_EQ("b'\\'\"'", starlark_bytes("'\"").str());
  EXPECT_EQ("b\'\\x00\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\t\\n\\x0b\\x0c\\r\\x0e\\x0f"
            "\\x10\\x11\\x12\\x13\\x14\\x15\\x16\\x17\\x18\\x19\\x1a\\x1b\\x1c\\x1d\\x1e\\x1f"
            " !\"#$%&\\\'()*+,-./0123456789:;<=>?"
            "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\\\]^_"
            "`abcdefghijklmnopqrstuvwxyz{|}~\\x7f"
            "\\x80\\x81\\x82\\x83\\x84\\x85\\x86\\x87\\x88\\x89\\x8a\\x8b\\x8c\\x8d\\x8e\\x8f"
            "\\x90\\x91\\x92\\x93\\x94\\x95\\x96\\x97\\x98\\x99\\x9a\\x9b\\x9c\\x9d\\x9e\\x9f"
            "\\xa0\\xa1\\xa2\\xa3\\xa4\\xa5\\xa6\\xa7\\xa8\\xa9\\xaa\\xab\\xac\\xad\\xae\\xaf"
            "\\xb0\\xb1\\xb2\\xb3\\xb4\\xb5\\xb6\\xb7\\xb8\\xb9\\xba\\xbb\\xbc\\xbd\\xbe\\xbf"
            "\\xc0\\xc1\\xc2\\xc3\\xc4\\xc5\\xc6\\xc7\\xc8\\xc9\\xca\\xcb\\xcc\\xcd\\xce\\xcf"
            "\\xd0\\xd1\\xd2\\xd3\\xd4\\xd5\\xd6\\xd7\\xd8\\xd9\\xda\\xdb\\xdc\\xdd\\xde\\xdf"
            "\\xe0\\xe1\\xe2\\xe3\\xe4\\xe5\\xe6\\xe7\\xe8\\xe9\\xea\\xeb\\xec\\xed\\xee\\xef"
            "\\xf0\\xf1\\xf2\\xf3\\xf4\\xf5\\xf6\\xf7\\xf8\\xf9\\xfa\\xfb\\xfc\\xfd\\xfe\\xff\'",
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
  EXPECT_FALSE(starlark_bytes("").truthy());
  EXPECT_TRUE(starlark_bytes("a").truthy());
}

TEST(StarlarkBytes, Equals) {
  EXPECT_TRUE(starlark_bytes("").equals(starlark_bytes("")));
  EXPECT_TRUE(starlark_bytes("a").equals(starlark_bytes("a")));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_bytes("a")));
  // This is the NFKC decomposition.
  EXPECT_FALSE(starlark_bytes("\u03C9\u0301").equals(starlark_bytes("\u03CE")));

  EXPECT_FALSE(starlark_bytes("").equals(starlark_bigint(0)));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_bool(false)));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_string("")));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_dictionary()));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_float(0.0)));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_function()));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_integer(0)));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_list()));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_none()));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_range()));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_set()));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_struct()));
  EXPECT_FALSE(starlark_bytes("").equals(starlark_tuple()));

  EXPECT_FALSE(starlark_bytes("0").equals(starlark_integer(0)));
}

TEST(StarlarkBytes, Hash) {
  EXPECT_EQ(0, starlark_bytes("").hash());
  EXPECT_EQ(0x5915dff8ab3f6382, starlark_bytes(std::string("\000", 1)).hash());
  EXPECT_EQ(0x47c474f5b32240c8, starlark_bytes("a").hash());
  EXPECT_EQ(0x69d6efa619d3857d, starlark_bytes("ab").hash());
  EXPECT_EQ(0x1f64c85d9a3c5aea, starlark_bytes("abc").hash());
  EXPECT_EQ(-0x0b5b097a9625e685, starlark_bytes("abcd").hash());
  EXPECT_EQ(0x168632fd4df59211, starlark_bytes("abcde").hash());
  EXPECT_EQ(-0x1316734e9ae39601, starlark_bytes("abcdef").hash());
  EXPECT_EQ(-0x046f814a39c03c2e, starlark_bytes("abcdefg").hash());
  EXPECT_EQ(-0x7fd30d59d72d058a, starlark_bytes("abcdefgh").hash());
  EXPECT_EQ(-0x6d11b9acae93c056, starlark_bytes("abcdefghi").hash());
  EXPECT_EQ(-0x045193c9a34bf07a, starlark_bytes("abcdefghij").hash());
  EXPECT_EQ(-0x493556008d0e92cb, starlark_bytes("abcdefghijk").hash());
  EXPECT_EQ(0x76ef23c6b008cd7b, starlark_bytes("abcdefghijkl").hash());
  EXPECT_EQ(-0x14a7e09195326e3d, starlark_bytes("abcdefghijklm").hash());
  EXPECT_EQ(0x033b8d82044dd206, starlark_bytes("abcdefghijklmn").hash());
  EXPECT_EQ(0x4b7d9ede3c051bec, starlark_bytes("abcdefghijklmno").hash());
  EXPECT_EQ(0x6ee54dcefca3d689, starlark_bytes("abcdefghijklmnop").hash());
  EXPECT_EQ(-0x5ac20331846b0484, starlark_bytes("abcdefghijklmnopq").hash());
  EXPECT_EQ(0x7b458279e2ffd51b, starlark_bytes("abcdefghijklmnopqr").hash());
  EXPECT_EQ(0x6d831f8a8e6a7c80, starlark_bytes("abcdefghijklmnopqrs").hash());
  EXPECT_EQ(-0x5221ec02394a14d7, starlark_bytes("abcdefghijklmnopqrst").hash());
  EXPECT_EQ(-0x4b38298791b754ea, starlark_bytes("abcdefghijklmnopqrstu").hash());
  EXPECT_EQ(0x40991f144590c002, starlark_bytes("abcdefghijklmnopqrstuv").hash());
  EXPECT_EQ(0x4db8fbf39f1ed05a, starlark_bytes("abcdefghijklmnopqrstuvw").hash());
  EXPECT_EQ(-0x251b735fced053c2, starlark_bytes("abcdefghijklmnopqrstuvwx").hash());
  EXPECT_EQ(-0xc0739218d5461ec, starlark_bytes("abcdefghijklmnopqrstuvwxy").hash());
  EXPECT_EQ(0x22b04466815fd83b, starlark_bytes("abcdefghijklmnopqrstuvwxyz").hash());
  EXPECT_EQ(-0x3cab5efd2f0468f4, starlark_bytes("abcdefghijklmnopqrstuvwxyz0").hash());
  EXPECT_EQ(-0x3b94d26b8d85c88a, starlark_bytes("abcdefghijklmnopqrstuvwxyz01").hash());
  EXPECT_EQ(0x4ab8bfbc363cd434, starlark_bytes("abcdefghijklmnopqrstuvwxyz012").hash());
  EXPECT_EQ(-0x557e62313da24871, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123").hash());
  EXPECT_EQ(-0x4fcc6fbe77b87973, starlark_bytes("abcdefghijklmnopqrstuvwxyz01234").hash());
  EXPECT_EQ(-0x151775372a5579d8, starlark_bytes("abcdefghijklmnopqrstuvwxyz012345").hash());
  EXPECT_EQ(0x6ffd51cc0fe5d3e, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456").hash());
  EXPECT_EQ(-0xe990dfb175a2aea, starlark_bytes("abcdefghijklmnopqrstuvwxyz01234567").hash());
  EXPECT_EQ(0x3f2df2d04784600b, starlark_bytes("abcdefghijklmnopqrstuvwxyz012345678").hash());
  EXPECT_EQ(-0x62f803d77a091faa, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456789").hash());
  EXPECT_EQ(-0x19fb4f2fd515d61, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456789@").hash());
  EXPECT_EQ(-0x77d493a040656198, starlark_bytes("abcdefghijklmnopqrstuvwxyz0123456789@!").hash());
}

TEST(StarlarkBytes, Order) {
  EXPECT_THAT(starlark_bytes("").cmp(starlark_bytes(""), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_bytes("").cmp(starlark_bytes("a"), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_bytes("a").cmp(starlark_bytes("a"), "cmp", nullptr), Eq(0));
  EXPECT_THAT(starlark_bytes("a").cmp(starlark_bytes(""), "cmp", nullptr), Gt(0));
  EXPECT_THAT(starlark_bytes("a").cmp(starlark_bytes("b"), "cmp", nullptr), Lt(0));
  EXPECT_THAT(starlark_bytes("b").cmp(starlark_bytes("a"), "cmp", nullptr), Gt(0));
}

TEST(StarlarkBytes, OrderErrors) {
  error_handler error_callback;
  EXPECT_FALSE(starlark_bytes("").cmp(starlark_string(""), "<", &error_callback));
  ASSERT_THAT(error_callback.messages, SizeIs(1));
  EXPECT_EQ(error_callback.messages[0], "TypeError: '<' not supported between instances of 'bytes' and 'string'");
}

TEST(StarlarkBytes, BinaryIn) {
  error_handler error_callback;
  EXPECT_TRUE(starlark_bytes("").binary_in(starlark_bytes(""), &error_callback));
  EXPECT_TRUE(starlark_bytes("a").binary_in(starlark_bytes(""), &error_callback));
  EXPECT_FALSE(starlark_bytes("a").binary_in(starlark_bytes("b"), &error_callback));
  EXPECT_FALSE(starlark_bytes("a").binary_in(starlark_integer('b'), &error_callback));
  EXPECT_TRUE(starlark_bytes("a").binary_in(starlark_integer('a'), &error_callback));
  EXPECT_TRUE(starlark_bytes("a").binary_in(starlark_bigint('a'), &error_callback));
  EXPECT_THAT(error_callback.messages, IsEmpty());
}

TEST(StarlarkBytes, BinaryInErrors) {
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_string(""), &error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "TypeError: a bytes-like object is required, not 'string'");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_integer(-1), &error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_integer(256), &error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_bigint(-1), &error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
  {
    error_handler error_callback;
    EXPECT_FALSE(starlark_bytes("").binary_in(starlark_bigint(256), &error_callback));
    ASSERT_THAT(error_callback.messages, SizeIs(1));
    EXPECT_EQ(error_callback.messages[0], "ValueError: byte must be in range(0, 256)");
  }
}

}  // namespace
