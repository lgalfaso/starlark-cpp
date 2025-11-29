// Copyright 2024-2025 Lucas Mirelmann

//
//  A class to handle arbitrary large integers.
//

#ifndef BIGINT_NUMBER_HPP_
#define BIGINT_NUMBER_HPP_

#include <cstdint>

#include <climits>
#include <compare>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace starlark {
namespace bigint {

class number {
 public:
  typedef uint64_t nbase;
  typedef std::vector<nbase> values_type;
  typedef std::vector<nbase>::size_type values_size_type;
  static constexpr int kBitsInBase = sizeof(nbase) * CHAR_BIT;
  static constexpr int kBaseHexSize = CHAR_BIT * sizeof(nbase) / 4;

 private:
  values_type values_;
  bool sign_;

 public:
  // Constructs a new instance of `number` with the default value of `0`.
  number();

  // Copy constructor.
  number(const number&) = default;

  // Move constructor.
  number(number&&) = default;

  explicit number(nbase value);

  // Returns whether the number is negative.
  bool sign() const;

  // Returns the length in blocks. A block represents `kBitsInBase` bits.
  values_size_type length() const;

  int bit_size() const;
  bool bit(int pos) const;
  nbase bits(int pos, int length) const;

  // Returns whether the number is even.
  bool even() const;

  // Returns an hexa representation of the number.
  std::string hex() const;
  std::string to_string(int base) const;

  // Negates this number.
  number& neg();

  // Compares two `number`. Returns a negative integer, zero, or a positive
  // integer as this object is less than, equal to, or greater than the
  // specified `number`.
  int cmp(const number& other) const;

  // Compare the absolute value between two `number`. Returns a negative
  // integer, zero, or a positive integer as this object absolute value is
  // less than, equal to, or greater than the absolute value of the specified
  // `number`.
  int abs_cmp(const number& other) const;

  // Common comparator operators.
  std::strong_ordering operator<=>(const number& other) const;
  bool operator==(const number& other) const;

  number& operator=(const number& other);
  number& operator=(number&& other);
  number& operator+=(const number& other);
  number operator+(const number& b) const &;
  number&& operator+(const number& b) &&;
  number& operator-=(const number& other);
  number operator-() const;
  number operator-(const number& b) const &;
  number&& operator-(const number& b) &&;
  number& operator*=(const number& other);
  number operator*(const number& b) const &;
  number&& operator*(const number& b) &&;
  number& operator%=(const number& other);
  number operator%(const number& b) const &;
  number&& operator%(const number& b) &&;
  number& operator/=(const number& other);
  number operator/(const number& b) const &;
  number&& operator/(const number& b) &&;

  number& operator>>=(int pos);
  number operator>>(int pos) const &;
  number&& operator>>(int pos) &&;
  number& operator<<=(int pos);
  number operator<<(int pos) const &;
  number&& operator<<(int pos) &&;

  number& pow_mod(const number& power, const number& modulus);

  static number parse_hex(std::string_view input);

  static std::tuple<number, number, number> gcd(const number& x,
                                                const number& y);

  static const number zero;
  static const number one;
  static const number minus_one;

  // Should not be used by end users.
  number& karatsuba(const number& other,
                    const values_size_type fallback_threshold);
  number& long_mult(const number& other);

  static std::pair<number, number> div(const number& dividend,
                                       const number& divisor);

  nbase at(values_size_type pos) const;
  int countr_zero() const;
  int countr_one() const;

  number& operator|=(const number& other);
  number operator|(const number& other) const &;
  number&& operator|(const number& other) &&;
  number& operator&=(const number& other);
  number operator&(const number& other) const &;
  number&& operator&(const number& other) &&;
  number& operator^=(const number& other);
  number operator^(const number& other) const &;
  number&& operator^(const number& other) &&;
  number operator~() const &;
  number&& operator~() &&;

  number& logical_or(const number& other);
  number& logical_and(const number& other);
  number& logical_xor(const number& other);
  number& logical_not();

 private:
  static void normalize(values_type* a);
  static bool cmp_values(const values_type& a, const values_type& b);
  static int abs_cmp(const values_type& a, const values_type& b);
  static void base_op(const values_type& a, const values_type& b,
                      values_type* c,
                      bool(&op)(const nbase a, const nbase b, nbase* to));
  static bool add_op(const nbase a, const nbase b, nbase* to);
  static bool dec_op(const nbase a, const nbase b, nbase* to);
  static void mult_op(const nbase a, const nbase b, nbase* high, nbase* low);
  static void long_mult(
    const values_type::const_iterator& a_begin,
    const values_type::const_iterator& a_end,
    const values_type::const_iterator& b_begin,
    const values_type::const_iterator& b_end,
    values_type* c);
  static void karatsuba_mult(
    const values_size_type fallback_size,
    const values_type::const_iterator& a_begin,
    const values_type::const_iterator& a_end,
    const values_type::const_iterator& b_begin,
    const values_type::const_iterator& b_end,
    values_type* c);
  static nbase inverse_mod_base(const nbase a);
  static number montgomery(const number& m, const nbase& inv_m, const number& x,
                           const number& y);
  number& mod_pow2(int power);
  void normalize();
  number& add_dec(const number& other, const bool is_add);
  void shift(int pos);
};


inline number operator""_number(const char* input, std::size_t size) {
  return number::parse_hex(std::string_view(input, size));
}

starlark::bigint::number parse_number(std::string_view input,
                                      const char** end_ptr);

}  // namespace bigint
}  // namespace starlark

#endif  // BIGINT_NUMBER_HPP_

