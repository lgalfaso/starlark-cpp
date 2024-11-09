// Copyright 2024 Lucas Mirelmann

#include "bignum/number.hpp"

#include <gtest/gtest.h>
#include <gtest/gtest-matchers.h>
#include <gmock/gmock.h>

using bignum::number;
using bignum::operator""_number;

namespace {

TEST(Number, StartsAtZero) {
  number num;
  EXPECT_EQ(0, num.length());
}

TEST(Number, SignIsFalseForZero) {
  number num;
  EXPECT_FALSE(num.sign());
}

TEST(Number, ConstructorUsingBase) {
  number num((number::base) 0x12345);
  EXPECT_EQ("0x12345", num.hex());
  EXPECT_EQ(1, num.length());
}

TEST(Number, ConstructorUsingBaseSetToZero) {
  number num((number::base) 0);
  EXPECT_EQ("0x0", num.hex());
  EXPECT_EQ(0, num.length());
}

TEST(Number, Negate) {
  number num_1((number::base) 0x1234);
  number num_2((number::base) 0);
  EXPECT_EQ("-0x1234", num_1.neg().hex());
  EXPECT_EQ("0x0", num_2.neg().hex());
}

TEST(Number, ParseHex) {
  std::string ref("1234567890abcdef1234567890abcdef");
  EXPECT_EQ("0x" + ref, number::parse_hex(ref).hex());
  EXPECT_EQ("-0x" + ref, number::parse_hex("-" + ref).hex());
  EXPECT_EQ("0x0", number::parse_hex("000000000000000000000000000000000"
                                     "00000000000").hex());
  EXPECT_EQ("0x10000000000000000", number::parse_hex(
      "10000000000000000").hex());
  EXPECT_EQ("0x123456789abcdef0123456789abcdef", "0x123456789abcdef0123456789ABCDEF"_number.hex());
}

TEST(Number, ParseInvalidHex) {
  EXPECT_EQ("0x0", "XXXXX"_number.hex());
  EXPECT_EQ("0x12345", "0x12345ggggggg"_number.hex());
  EXPECT_EQ("0x12345012345", "0x12345ggggggg012345"_number.hex());
}

TEST(Number, bit_size) {
  EXPECT_EQ(0, number::zero.bit_size());
  number n = number::one;
  for (int i = 1; i <= number::kBitsInBase * 2; ++i) {
    EXPECT_EQ(i, n.bit_size());
    n <<= 1;
  }
}

TEST(Number, Comparators) {
  std::vector<number> numbers;
  numbers.push_back("-fedcba0987654321fedcba0987654321"_number);
  numbers.push_back("-fedcba09876543211234567890abcdef"_number);
  numbers.push_back("-1234567890abcdeffedcba0987654321"_number);
  numbers.push_back("-1234567890abcdef1234567890abcdef"_number);
  numbers.push_back("-fedcba0987654321"_number);
  numbers.push_back("-1234567890abcdef"_number);
  numbers.push_back("0"_number);
  numbers.push_back("1234567890abcdef"_number);
  numbers.push_back("fedcba0987654321"_number);
  numbers.push_back("1234567890abcdef1234567890abcdef"_number);
  numbers.push_back("1234567890abcdeffedcba0987654321"_number);
  numbers.push_back("fedcba09876543211234567890abcdef"_number);
  numbers.push_back("fedcba0987654321fedcba0987654321"_number);
  for (std::vector<number>::size_type i = 0; i < numbers.size(); ++i) {
    for (std::vector<number>::size_type j = 0; j < numbers.size(); ++j) {
      EXPECT_EQ(i == j, numbers[i] == numbers[j]);
      EXPECT_EQ(i != j, numbers[i] != numbers[j]);
      EXPECT_EQ(i < j, numbers[i] < numbers[j]);
      EXPECT_EQ(i <= j, numbers[i] <= numbers[j]);
      EXPECT_EQ(i > j, numbers[i] > numbers[j]);
      EXPECT_EQ(i >= j, numbers[i] >= numbers[j]);
    }
  }
}

TEST(Number, AddingToSelf) {
  EXPECT_EQ("1fdb974130eca8643fdb974130eca8642"_number,
            "fedcba0987654321fedcba0987654321"_number +=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("0"_number,
            "-fedcba0987654321fedcba0987654321"_number +=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("0"_number,
            "fedcba0987654321fedcba0987654321"_number +=
            "-fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-1fdb974130eca8643fdb974130eca8642"_number,
            "-fedcba0987654321fedcba0987654321"_number +=
            "-fedcba0987654321fedcba0987654321"_number);

  EXPECT_EQ("fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number +=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number +=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number +=
            "-fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number +=
            "-fedcba0987654321fedcba0987654321"_number);

  EXPECT_EQ("fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "fedcba0987654321fedcba0987654321"_number +=
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);
  EXPECT_EQ("fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "-fedcba0987654321fedcba0987654321"_number +=
            "fedcba0987654321fedcba09876543"
            "2112345678901234567890"_number);
  EXPECT_EQ("-fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "fedcba0987654321fedcba0987654321"_number +=
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);
  EXPECT_EQ("-fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "-fedcba0987654321fedcba0987654321"_number +=
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);

  EXPECT_EQ("10000000000000000000000000000000"
            "000000000000000000000000000000000"_number,
            "ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number +=
            "1"_number);
  EXPECT_EQ("10000000000000000000000000000000"
            "000000000000000000000000000000000"_number,
            "1"_number +=
            "ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number);
  EXPECT_EQ("ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number,
            "10000000000000000000000000000000"
            "000000000000000000000000000000000"_number +=
            "-1"_number);
  EXPECT_EQ("ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number,
            "-1"_number +=
            "10000000000000000000000000000000"
            "000000000000000000000000000000000"_number);
}

TEST(Number, AddingSelfToSelf) {
  number num = "fedcba0987654321fedcba0987654321"_number;
  num += num;
  EXPECT_EQ("0x1fdb974130eca8643fdb974130eca8642"_number, num);
}

TEST(Number, DecrementingToSelf) {
  EXPECT_EQ("0"_number,
            "fedcba0987654321fedcba0987654321"_number -=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-1fdb974130eca8643fdb974130eca8642"_number,
            "-fedcba0987654321fedcba0987654321"_number -=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("1fdb974130eca8643fdb974130eca8642"_number,
            "fedcba0987654321fedcba0987654321"_number -=
            "-fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("0"_number,
            "-fedcba0987654321fedcba0987654321"_number -=
            "-fedcba0987654321fedcba0987654321"_number);

  EXPECT_EQ("fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number -=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number -=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number -=
            "-fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number -=
            "-fedcba0987654321fedcba0987654321"_number);

  EXPECT_EQ("-fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "fedcba0987654321fedcba0987654321"_number -=
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);
  EXPECT_EQ("-fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "-fedcba0987654321fedcba0987654321"_number -=
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);
  EXPECT_EQ("fedcba0987654321feddb8e6416eca86"
            "555655554a1bbbbbbbb1"_number,
            "fedcba0987654321fedcba0987654321"_number -=
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);
  EXPECT_EQ("fedcba0987654321fedbbb2ccd5bbbbb"
            "cf12579bd608acf1356f"_number,
            "-fedcba0987654321fedcba0987654321"_number -=
            "-fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);

  EXPECT_EQ("10000000000000000000000000000000"
            "000000000000000000000000000000000"_number,
            "ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number -=
            "-1"_number);
  EXPECT_EQ("10000000000000000000000000000000"
            "000000000000000000000000000000000"_number,
            "1"_number -=
            "-ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number);
  EXPECT_EQ("ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number,
            "10000000000000000000000000000000"
            "000000000000000000000000000000000"_number -=
            "1"_number);
  EXPECT_EQ("ffffffffffffffffffffffffffffffff"
            "ffffffffffffffffffffffffffffffff"_number,
            "-1"_number -=
            "-10000000000000000000000000000000"
            "000000000000000000000000000000000"_number);
}

TEST(Number, DecrementingSelfToSelf) {
  number num = "fedcba0987654321fedcba0987654321"_number;
  num -= num;
  EXPECT_EQ("0x0"_number, num);
}

TEST(Number, MultiplicationToSelf) {
  EXPECT_EQ("fdbabf7b303f805cca74202338234af7"
            "9bb801d4df8814dccefea12cd7a44a41"_number,
            "fedcba0987654321fedcba0987654321"_number *=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-fdbabf7b303f805cca74202338234af7"
            "9bb801d4df8814dccefea12cd7a44a41"_number,
            "-fedcba0987654321fedcba0987654321"_number *=
            "fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("-fdbabf7b303f805cca74202338234af7"
            "9bb801d4df8814dccefea12cd7a44a41"_number,
            "fedcba0987654321fedcba0987654321"_number *=
            "-fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("fdbabf7b303f805cca74202338234af7"
            "9bb801d4df8814dccefea12cd7a44a41"_number,
            "-fedcba0987654321fedcba0987654321"_number *=
            "-fedcba0987654321fedcba0987654321"_number);
  EXPECT_EQ("0"_number,
            "fedcba0987654321fedcba0987654321"_number *=
            "0"_number);

  EXPECT_EQ("fdbabf7b303f805cca74202338234af7"
            "add7a1d58261645faea28b97ed4a7b47"
            "08144a6a72cce1833a90"_number,
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number *=
            "fedcba0987654321fedcba0987654321"_number);

  EXPECT_EQ("fdbabf7b303f805cca74202338234af7"
            "add7a1d58261645faea28b97ed4a7b47"
            "08144a6a72cce1833a90"_number,
            "fedcba0987654321fedcba0987654321"_number *=
            "fedcba0987654321fedcba0987654321"
            "12345678901234567890"_number);
}

TEST(Number, Karatsuba) {
  const int length = 100;
  std::string block = "1234567890abcdef";
  std::string hex_number;
  hex_number.reserve(length * block.size());
  for (int i = 0; i < length; ++i) {
    hex_number += block;
  }
  number number_long = number::parse_hex(hex_number);
  number_long.long_mult(number_long);
  number number_k = number::parse_hex(hex_number);
  for (int i = 1; i < length; i += 10) {
    number number_k = number::parse_hex(hex_number);
    number_k.karatsuba(number_k, i);
    EXPECT_EQ(number_long, number_k);
  }
}

class NumberBenchmarkTest : public ::testing::TestWithParam<int> {};
TEST_P(NumberBenchmarkTest, Multiplication) {
  const int length = 10000;
  std::string block = "1234567890abcdef";
  std::string hex_number;
  hex_number.reserve(length * block.size());
  for (int i = 0; i < length; ++i) {
    hex_number += block;
  }
  number num = number::parse_hex(hex_number);
  num.karatsuba(num, GetParam());
}

INSTANTIATE_TEST_SUITE_P(KaratsubaBig, NumberBenchmarkTest,
                         ::testing::Range(1, 10002, 500));
INSTANTIATE_TEST_SUITE_P(KaratsubaMid, NumberBenchmarkTest,
                         ::testing::Range(1, 502, 10));
INSTANTIATE_TEST_SUITE_P(KaratsubaSmall, NumberBenchmarkTest,
                         ::testing::Range(1, 150));

TEST(Number, Shift) {
  EXPECT_EQ("1234567890abcdef00"_number,
            "1234567890abcdef0"_number << 4);
  EXPECT_EQ("1234567890abcdef00000000000000000"_number,
            "1234567890abcdef0"_number << 64);
  EXPECT_EQ("1234567890abcdef000000000000000000"_number,
            "1234567890abcdef0"_number << 68);
  EXPECT_EQ("1234567890abcdef"_number,
            "1234567890abcdef0"_number >> 4);
  EXPECT_EQ("123456789"_number,
            "1234567890abcdef000000000"_number >> 64);
  EXPECT_EQ("123456789"_number,
            "1234567890abcdef0000000000"_number >> 68);

  EXPECT_EQ("1234567890abcdef00"_number,
            "1234567890abcdef0"_number >> -4);
  EXPECT_EQ("1234567890abcdef00000000000000000"_number,
            "1234567890abcdef0"_number >> -64);
  EXPECT_EQ("1234567890abcdef000000000000000000"_number,
            "1234567890abcdef0"_number >> -68);
  EXPECT_EQ("1234567890abcdef"_number,
            "1234567890abcdef0"_number << -4);
  EXPECT_EQ("123456789"_number,
            "1234567890abcdef000000000"_number << -64);
  EXPECT_EQ("123456789"_number,
            "1234567890abcdef0000000000"_number << -68);
}

TEST(Number, GCD) {
  number num_1 = "1234567890abcdef0"_number;
  number num_2 = "fedcba09876543210"_number;
  number a, b, g;
  std::tie(a, b, g) = number::gcd(num_1, num_2);
  EXPECT_NE("0x0"_number, a);
  EXPECT_NE("0x0"_number, b);
  EXPECT_EQ("0xf0"_number, g);
  EXPECT_EQ(g, a * num_1 + b * num_2);
}

// `2*(10^1000-1)/9 + 30051`
static char kBigPrime[] =
    "d87df388c4a898e86b65db2f4fc430bb46c8d7b5edd26540de6d8b6ba0d305eb20fb"
    "5fcaeb1318daa769f61a84917c4eab63c81fdb08b26e7b39e5fef2b7c1f21ff34f97"
    "fdcc4212f590a37e4a50d076c23a1732b0cde504953e4db384127f1577de23c12b47"
    "ef768007adb034832a22fcadf91c1a8b3f7165058ad57215e3b255fa0d2832f90721"
    "6a12f5d0a5e1e3074646d30aa8d9e73d49ac7fc4b82f0655ef051af826973b98d35f"
    "82d247f617028f815cb182c88ce39137064085b0666a018d5b397746ffdfa046103e"
    "8e97cc868270383a9a0497626930c191468c4a650165ced61e13cd29ba94afbd82c5"
    "077f42259efed91dabb2c8c48c875f861771dc1b9981253ca0cd4993d72d2f0338eb"
    "19da2808a4da1fe980937a86f329fa8a416ae38e38e38e38e38e38e38e38e38e38e3"
    "8e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e"
    "38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38"
    "e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e38e3"
    "8e38e38e3958f1";

static char kBigNumber[] =
    "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"
    "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"
    "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef"
    "1234567890abcdef1234567890abcdef1234567890abcdef1234567890abcdef";

TEST(Number, DivisionSmall) {
  for (int i = 0; i < 4; ++i) {
    number n = number::one << 40;
    number p = "fedcba"_number;
    if ((i & 1) != 0) n.neg();
    if ((i & 2) != 0) p.neg();
    number d, r;
    std::tie(d, r) = number::div(n, p);
    if (!r.sign()) {
      EXPECT_TRUE(number::zero <= r);
    } else {
      EXPECT_TRUE(r <= number::zero);
    }
    EXPECT_LT(r.abs_cmp(p), 0);
    EXPECT_EQ(n, d * p + r);
  }
}

TEST(Number, DivisionBig) {
  for (int j = 4; j < 16; ++j) {
    for (int i = 0; i < 4; ++i) {
      number n = number::one << (sizeof(kBigPrime) * j);
      number p = number::parse_hex(kBigPrime);
      if ((i & 1) != 0) n.neg();
      if ((i & 2) != 0) p.neg();
      number d, r;
      std::tie(d, r) = number::div(n, p);
      if (!r.sign()) {
        EXPECT_TRUE(number::zero <= r);
      } else {
        EXPECT_TRUE(r <= number::zero);
      }
      EXPECT_LT(r.abs_cmp(p), 0);
      EXPECT_EQ(n, d * p + r);
    }
  }
}

TEST(Number, Mod) {
  number n = number::one << sizeof(kBigPrime) * 8;
  number p = number::parse_hex(kBigPrime);
  number r;
  r = n % p;
}

TEST(Number, ModPowSimple) {
  number p = number::parse_hex(kBigPrime);
  number r = number::parse_hex(kBigNumber);
  EXPECT_EQ(number::one, r.pow_mod(p - number::one, p));
  EXPECT_EQ(number::one, ("0x1"_number << 10000).pow_mod(p - number::one, p));
  EXPECT_EQ(number::one, (-"0x1"_number << 10000).pow_mod(p - number::one, p));
}

TEST(Number, ModPowSimpleEvenMod) {
  number p = number::parse_hex(kBigPrime);
  number r = number::parse_hex(kBigNumber);
  number::base power2 = 50;
  EXPECT_EQ(number::one, r.pow_mod((p - number::one) * (number::one << power2), p * (number::one << (power2 - 1))));
}

TEST(Number, ModPowSimplePower2Mod) {
  number r = number::parse_hex(kBigNumber);
  number::base power2 = 500;
  EXPECT_EQ(number::one, r.pow_mod(number::one << power2, number::one << (power2 - 1)));
}

TEST(Number, ModPow) {
  number p = number::parse_hex(kBigPrime);
  number e = "10001"_number;
  number phi = p - number::one;
  number inv_e_phi;
  std::tie(inv_e_phi, std::ignore, std::ignore) = number::gcd(e, phi);
  number r = number::parse_hex(kBigNumber);
  number r0 = r;
  r.pow_mod(e, p);
  EXPECT_NE(r0, r);
  r.pow_mod(inv_e_phi, p);
  EXPECT_EQ(r0, r);
}

}  // namespace
