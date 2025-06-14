// Copyright 2024-2025 Lucas Mirelmann

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

#include "runtime/siphash.hpp"

using starlark::runtime::siphash;

namespace {

TEST(Siphash, Hash) {
  EXPECT_EQ(0xd1fba762150c532c, siphash("", 0, 0, 0));
  EXPECT_EQ(0x72873bfd1bca2911, siphash("", 0, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x5915dff8ab3f6382, siphash("\000", 1, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x47c474f5b32240c8, siphash("a", 1, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x69d6efa619d3857d, siphash("ab", 2, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x1f64c85d9a3c5aea, siphash("abc", 3, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x0b5b097a9625e685, siphash("abcd", 4, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x168632fd4df59211, siphash("abcde", 5, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x1316734e9ae39601, siphash("abcdef", 6, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x046f814a39c03c2e, siphash("abcdefg", 7, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x7fd30d59d72d058a, siphash("abcdefgh", 8, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x6d11b9acae93c056, siphash("abcdefghi", 9, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x045193c9a34bf07a, siphash("abcdefghij", 10, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x493556008d0e92cb, siphash("abcdefghijk", 11, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x76ef23c6b008cd7b, siphash("abcdefghijkl", 12, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x14a7e09195326e3d, siphash("abcdefghijklm", 13, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x033b8d82044dd206, siphash("abcdefghijklmn", 14, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x4b7d9ede3c051bec, siphash("abcdefghijklmno", 15, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x6ee54dcefca3d689, siphash("abcdefghijklmnop", 16, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x5ac20331846b0484, siphash("abcdefghijklmnopq", 17, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x7b458279e2ffd51b, siphash("abcdefghijklmnopqr", 18, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x6d831f8a8e6a7c80, siphash("abcdefghijklmnopqrs", 19, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x5221ec02394a14d7, siphash("abcdefghijklmnopqrst", 20, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x4b38298791b754ea, siphash("abcdefghijklmnopqrstu", 21, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x40991f144590c002, siphash("abcdefghijklmnopqrstuv", 22, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x4db8fbf39f1ed05a, siphash("abcdefghijklmnopqrstuvw", 23, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x251b735fced053c2, siphash("abcdefghijklmnopqrstuvwx", 24, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0xc0739218d5461ec, siphash("abcdefghijklmnopqrstuvwxy", 25, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x22b04466815fd83b, siphash("abcdefghijklmnopqrstuvwxyz", 26, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x3cab5efd2f0468f4, siphash("abcdefghijklmnopqrstuvwxyz0", 27, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x3b94d26b8d85c88a, siphash("abcdefghijklmnopqrstuvwxyz01", 28, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x4ab8bfbc363cd434, siphash("abcdefghijklmnopqrstuvwxyz012", 29, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x557e62313da24871, siphash("abcdefghijklmnopqrstuvwxyz0123", 30, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x4fcc6fbe77b87973, siphash("abcdefghijklmnopqrstuvwxyz01234", 31, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x151775372a5579d8, siphash("abcdefghijklmnopqrstuvwxyz012345", 32, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x6ffd51cc0fe5d3e, siphash("abcdefghijklmnopqrstuvwxyz0123456", 33, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0xe990dfb175a2aea, siphash("abcdefghijklmnopqrstuvwxyz01234567", 34, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(0x3f2df2d04784600b, siphash("abcdefghijklmnopqrstuvwxyz012345678", 35, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x62f803d77a091faa, siphash("abcdefghijklmnopqrstuvwxyz0123456789", 36, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x19fb4f2fd515d61, siphash("abcdefghijklmnopqrstuvwxyz0123456789@", 37, 0x0001020304050607, 0x08090a0b0c0d0e0f));
  EXPECT_EQ(-0x77d493a040656198, siphash("abcdefghijklmnopqrstuvwxyz0123456789@!", 38, 0x0001020304050607, 0x08090a0b0c0d0e0f));
}

}  // namespace
