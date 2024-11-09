// Copyright 2024 Lucas Mirelmann

//
//  A class to handle arbitrary large integers.
//

#ifndef BIGNUM_NUMBER_HPP_
#define BIGNUM_NUMBER_HPP_

#include <climits>
#include <compare>
#include <ostream>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

namespace bignum {

class number {
 public:
  typedef uint64_t base;
  typedef std::vector<base> values_type;
  typedef std::vector<base>::size_type values_size_type;
  static constexpr int kBitsInBase = sizeof(base) * CHAR_BIT;
  static constexpr int kBaseHexSize = CHAR_BIT * sizeof(base) / 4;

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

  explicit number(base base_value);

  // Returns whether the number is negative.
  bool sign() const;

  // Returns the length in blocks. A block represents `kBitsInBase` bits.
  values_size_type length() const;

  int bit_size() const;
  bool bit(int pos) const;
  base bits(int pos, int length) const;

  // Returns whether the number is even.
  bool even() const;

  // Returns an hexa representation of the number.
  std::string hex() const;

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
  number operator+(const number& b) const;
  number& operator-=(const number& other);
  number& operator-();
  number operator-(const number& b) const;
  number& operator*=(const number& other);
  number operator*(const number& b) const;
  number& operator%=(const number& other);
  number operator%(const number& b) const;

  number& operator>>=(int pos);
  number operator>>(int pos) const;
  number& operator<<=(int pos);
  number operator<<(int pos) const;

  number& pow_mod(const number& power, const number& modulus);

  static number parse_hex(std::string_view input);

  static std::tuple<number, number, number> gcd(const number& x,
                                                const number& y);

  static const number zero;
  static const number one;

  // Should not be used by end users.
  number& karatsuba(const number& other,
                    const values_size_type fallback_threshold);
  number& long_mult(const number& other);

  static std::pair<number, number> div(const number& dividend,
                                       const number& divisor);

 private:
  base at(values_size_type pos) const;
  int countr_zero() const;
  static void normalize(values_type* a);
  static bool cmp_values(const values_type& a, const values_type& b);
  static int abs_cmp(const values_type& a, const values_type& b);
  static void base_op(const values_type& a, const values_type& b,
                      values_type* c,
                      bool(&op)(const base a, const base b, base* to));
  static bool add_op(const base a, const base b, base* to);
  static bool dec_op(const base a, const base b, base* to);
  static void mult_op(base a, base b, base* high, base* low);
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
  static base inverse_mod_base(const base a);
  static number montgomery(const number& m, const base& inv_m, const number& x,
                           const number& y);
  number& mod_pow2(int power);
  void normalize();
  number& add_dec(const number& other, const bool is_add);
  void shift(int pos);
};

std::ostream& operator<<(std::ostream& os, const number& other);

inline number operator""_number(const char* input, std::size_t size) {
  return number::parse_hex(std::string_view(input, size));
}

}  // namespace bignum

#endif  // BIGNUM_NUMBER_HPP_

