// Copyright 2024-2025 Lucas Mirelmann

#include "bignum/number.hpp"

#include <cassert>

#include <algorithm>
#include <bit>
#include <iomanip>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace bignum {

namespace {

int exponentiation_mask_size(int bit_size) {
  // Based on the number of bits in the power, and the mask size, the number of
  // operations is about `bits - 1 + std::ceil(bits/(mask_size + 0.5)) + pow(2, mask_size - 1) - 1`.
  // This formula assumes that the number of `1`s and `0`s in the power
  // are about the same and in random positions.
  // If this holds true, then the mask size is minimized using the constants below.

  // There is no need to do anything fancier, this is very cheap compared to an exponentiation.
  if (bit_size < 64) {
    return 1;
  }
  if (bit_size < 128) {
    return 3;
  }
  if (bit_size < 256) {
    return 4;
  }
  if (bit_size < 640) {
    return 5;
  }
  if (bit_size < 1600) {
    return 6;
  }
  if (bit_size < 4096) {
    return 7;
  }
  if (bit_size < 10432) {
    return 8;
  }
  if (bit_size < 25664) {
    return 9;
  }
  if (bit_size < 61888) {
    return 10;
  }
  if (bit_size < 147328) {
    return 11;
  }
  return 12;
}

}  // namespace

number::number() : sign_(false) {}

number::number(number::nbase value) : sign_(false) {
  if (value != 0) {
    values_.push_back(value);
  }
}

number::nbase number::at(values_size_type pos) const {
  if (values_.size() <= pos) {
    return nbase{0};
  }
  return values_[pos];
}

int number::countr_zero() const {
  int result = 0;
  for (const auto& element : values_) {
    if (element != 0) {
      result += std::countr_zero(element);
      break;
    }
    result += kBitsInBase;
  }
  return result;
}

int number::bit_size() const {
  if (values_.empty()) return 0;
  return values_.size() * kBitsInBase - std::countl_zero<nbase>(values_.back());
}

bool number::bit(int pos) const {
  if (pos < 0) return false;
  const auto r = at(pos / kBitsInBase);
  return ((r >> (pos % kBitsInBase)) & 1) == 1;
}

number::nbase number::bits(int pos, int length) const {
  if (pos < 0 || length <= 0) return nbase{0};
  int base_pos = pos / kBitsInBase;
  int rem_pos = pos % kBitsInBase;
  nbase b;
  if (rem_pos == 0) {
    b = at(base_pos);
  } else {
    b = (at(base_pos) >> rem_pos) |
        (at(base_pos + 1) << (kBitsInBase - rem_pos));
  }
  if (length >= kBitsInBase) return b;
  return ((nbase{1} << length) - nbase{1}) & b;
}

// static const.
const number number::zero(0);

// static const.
const number number::one(1);

number& number::operator=(const number& other) {
  values_ = other.values_;
  sign_ = other.sign_;
  return *this;
}

number& number::operator=(number&& other) {
  std::swap(values_, other.values_);
  std::swap(sign_, other.sign_);
  return *this;
}

number::values_size_type number::length() const {
  return values_.size();
}

bool number::even() const {
  return (at(0) & 1) == 0;
}

bool number::sign() const {
  return sign_;
}

std::string number::hex() const {
  if (values_.empty()) {
    return "0x0";
  }
  std::stringstream ss;
  if (sign_) {
    ss << "-";
  }
  ss << "0x" << std::hex;
  for (auto it = values_.crbegin(); it != values_.crend(); ++it) {
    ss << *it << std::setfill('0') << std::setw(kBaseHexSize);
  }
  return ss.str();
}

std::string number::to_string(int base) const {
  static const char nums[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  if (base < 2 || base > 36) {
    return "";
  }
  std::string result;
  if (*this == number::zero) {
    result = "0";
    return result;
  }
  number ref(*this);
  bool neg = false;
  if (ref.sign()) {
    neg = true;
    ref.neg();
  }
  number num_base(base);
  while (ref != number::zero) {
    const auto [res, rem] = number::div(ref, num_base);
    ref = res;
    result = nums[rem.at(0)] + result;
  }
  if (neg) {
    result = "-" + result;
  }
  return result;
}

number& number::neg() {
  sign_ = !sign_ && !values_.empty();
  return *this;
}

// static.
int number::abs_cmp(const values_type& a, const values_type& b) {
  if (a.size() != b.size()) {
    return a.size() < b.size() ? -1 : 1;
  }
  for (auto it1 = a.crbegin(), it2 = b.crbegin(); it1 != a.crend();
       ++it1, ++it2) {
    if (*it1 != *it2) {
      return *it1 < *it2 ? -1 : 1;
    }
  }
  return 0;
}

int number::abs_cmp(const number& other) const {
  return abs_cmp(values_, other.values_);
}

int number::cmp(const number& other) const {
  int multiplier = sign_ ? -1 : 1;
  if (sign_ != other.sign_) {
    return multiplier;
  }
  return multiplier * abs_cmp(other);
}

std::strong_ordering number::operator<=>(const number& other) const {
  auto result = cmp(other);
  if (result == 0) {
    return std::strong_ordering::equal;
  }
  if (result < 0) {
    return std::strong_ordering::less;
  }
  return std::strong_ordering::greater;
}

bool number::operator==(const number& other) const {
  return cmp(other) == 0;
}

number::nbase parse_hex_digit(const char& input) {
  if ('0' <= input && input <= '9') {
    return input - '0';
  }
  if ('a' <= input && input <= 'f') {
    return input - 'a' + 10;
  }
  if ('A' <= input && input <= 'F') {
    return input - 'A' + 10;
  }
  return 0xff;
}

// static.
void number::normalize(values_type* a) {
  while (!a->empty() && a->back() == 0) {
    a->pop_back();
  }
}

void number::normalize() {
  normalize(&values_);
  sign_ &= !values_.empty();
}

number& number::mod_pow2(int power) {
  assert(power >= 0);
  if (power == 0) {
    *this = zero;
    return *this;
  }
  sign_ = false;
  if (bit_size() <= power) {
    return *this;
  }
  values_.resize((power + kBitsInBase - 1) / kBitsInBase);
  values_.back() &= (~nbase{0}) >> (kBitsInBase - 1 - (power + kBitsInBase -1) % kBitsInBase);
  normalize();
  return *this;
}

// static
number number::parse_hex(std::string_view input) {
  number result;
  int partial_hex_size = 0;
  nbase partial = 0;
  for (auto it = input.crbegin(); it != input.crend(); ++it) {
    nbase hex_value = parse_hex_digit(*it);
    if (hex_value != 0xff) {
      partial += (hex_value << (4 * partial_hex_size));
      partial_hex_size++;
      if (partial_hex_size == kBaseHexSize) {
        result.values_.push_back(partial);
        partial = 0;
        partial_hex_size = 0;
      }
    } else if (*it == '-') {
      result.sign_ = true;
    }
  }
  if (partial != 0) {
    result.values_.push_back(partial);
  }
  result.normalize();
  return result;
}

// static.
bool number::add_op(const nbase a, const nbase b, nbase* to) {
  *to = a + b;
  return a > *to;
}

// static.
bool number::dec_op(const nbase a, const nbase b, nbase* to) {
  *to = a - b;
  return a < *to;
}

// static.
bool number::cmp_values(const values_type& a, const values_type& b) {
  return number::abs_cmp(a, b) < 0;
}

// static.
void number::base_op(const values_type& a, const values_type& b, values_type* c,
                     bool(&op)(const nbase a, const nbase b, nbase* to)) {
  const auto& [small, big] = std::minmax(a, b, cmp_values);
  const auto size_small = small.size();
  const auto size_big = big.size();
  c->resize(size_big, 0);
  const auto* data_small = small.data();
  const auto* data_big = big.data();
  auto* data_c = c->data();
  values_size_type i;
  nbase carry = 0;
  for (i = 0; i < size_small; ++i) {
    nbase new_carry = 0;
    if (op(data_big[i], data_small[i], &data_c[i])) {
      ++new_carry;
    }
    if (carry != 0 && op(data_c[i], carry, &data_c[i])) {
      ++new_carry;
    }
    carry = new_carry;
  }
  for (; carry != 0 && i < size_big; ++i) {
    carry = op(data_big[i], carry, &data_c[i]);
  }
  for (; i < size_big; ++i) {
    data_c[i] = data_big[i];
  }
  if (carry != 0) {
    c->emplace_back(carry);
  }
}

number& number::add_dec(const number& other, const bool is_add) {
  if (is_add == (sign_ == other.sign_)) {
    base_op(values_, other.values_, &values_, add_op);
  } else {
    sign_ ^= cmp_values(values_, other.values_);
    base_op(values_, other.values_, &values_, dec_op);
    normalize();
  }
  return *this;
}

number& number::operator+=(const number& other) {
  return add_dec(other, true);
}

number& number::operator-=(const number& other) {
  return add_dec(other, false);
}

number number::operator+(const number& other) const {
  number result(*this);
  return result += other;
}

number number::operator-() const {
  number result(*this);
  return result.neg();
}

number number::operator-(const number& other) const {
  number result(*this);
  return result -= other;
}

// static.
void number::mult_op(const nbase a, const nbase b, nbase* high, nbase* low) {
  using u128 = unsigned __int128;
  u128 result = u128(a) * b;
  *low = result;
  *high = result >> 64;
}

// static.
void number::long_mult(
    const values_type::const_iterator& a_begin,
    const values_type::const_iterator& a_end,
    const values_type::const_iterator& b_begin,
    const values_type::const_iterator& b_end,
    values_type* c) {
  const auto a_size = a_end - a_begin;
  const auto b_size = b_end - b_begin;
  values_type mult(a_size + b_size, 0);
  values_type carry(a_size + b_size, 0);
  for (values_size_type i = 0; i < a_size; ++i) {
    for (values_size_type j = 0; j < b_size; ++j) {
      nbase high, low;
      mult_op(a_begin[i], b_begin[j], &high, &low);
      if (add_op(mult[i+j], low, &mult[i+j])) {
        ++carry[i+j+1];
      }
      if (add_op(mult[i+j+1], high, &mult[i+j+1])) {
        ++carry[i+j+2];
      }
    }
  }
  base_op(mult, carry, c, add_op);
  normalize(c);
}


number& number::operator*=(const number& other) {
  static const values_size_type karatsuba_threshold = 128;
  return karatsuba(other, karatsuba_threshold);
}

number number::operator*(const number& other) const {
  number result(*this);
  return result *= other;
}

// static.
std::pair<number, number> number::div(const number& dividend,
                                      const number& divisor) {
  int s_shift = kBitsInBase;
  if (divisor == zero) {
    return std::make_pair(zero, zero);
  }
  if (abs_cmp(divisor.values_, one.values_) == 0) {
    return std::make_pair(dividend, zero);
  }
  if (abs_cmp(dividend.values_, divisor.values_) < 0) {
    return std::make_pair(zero, dividend);
  }
  number dd = divisor << s_shift;
  number d, r = dividend << s_shift;
  int divisor_bit_size = dd.bit_size();
  int dividend_bit_size;
  int r_d = std::max(0, divisor_bit_size - (kBitsInBase >> 1));
  nbase factor = dd.bits(r_d, kBitsInBase >> 1) + 1;
  int bits_in_factor = kBitsInBase - std::countl_zero(factor);
  while ((dividend_bit_size = r.bit_size()) > divisor_bit_size) {
    int r_s = std::max(0, dividend_bit_size - kBitsInBase);
    nbase base_divisor = r.bits(r_s, kBitsInBase);
    nbase base_division = base_divisor / factor;
    number base_division_number(base_division);
    int shift_factor = (dividend_bit_size - (kBitsInBase - std::countl_zero(base_divisor))) -
                       (divisor_bit_size - bits_in_factor);
    number base_division_times_dd;
    if (shift_factor > 0) {
      base_division_times_dd = (dd * base_division_number) << shift_factor;
      base_division_number <<= shift_factor;
    } else {
      base_division_number <<= shift_factor;
      base_division_times_dd = dd * base_division_number;
    }
    if (abs_cmp(r.values_, base_division_times_dd.values_) < 0) {
      if ((base_division_number.at(0) & 1) != 0) {
        base_division_times_dd -= dd;
      }
      base_division_number >>= 1;
      base_division_times_dd >>= 1;
    }
    base_op(d.values_, base_division_number.values_, &d.values_, add_op);
    d.normalize();

    base_op(r.values_, base_division_times_dd.values_, &r.values_,
            dec_op);
    r.normalize();
  }
  while (abs_cmp(r.values_, dd.values_) >= 0) {
    base_op(r.values_, dd.values_, &r.values_, dec_op);
    base_op(d.values_, one.values_, &d.values_, add_op);
    r.normalize();
    d.normalize();
  }
  d.sign_ = dividend.sign_ ^ dd.sign_;
  return std::make_pair(d, r >> s_shift);
}

number& number::operator%=(const number& other) {
  std::tie(std::ignore, *this) = div(*this, other);
  return *this;
}

number number::operator%(const number& other) const {
  number result(*this);
  return result %= other;
}

number& number::long_mult(const number& other) {
  long_mult(values_.cbegin(), values_.cend(), other.values_.cbegin(),
            other.values_.cend(), &values_);
  sign_ ^= other.sign_;
  return *this;
}

// static.
void number::karatsuba_mult(
    const values_size_type fallback_threshold,
    const values_type::const_iterator& a_begin,
    const values_type::const_iterator& a_end,
    const values_type::const_iterator& b_begin,
    const values_type::const_iterator& b_end,
    values_type* c) {
  const auto a_size = a_end - a_begin;
  const auto b_size = b_end - b_begin;
  const auto [min, max] = std::minmax(a_size, b_size);
  const auto half_max = (max + 1) / 2;
  if (min <= fallback_threshold) {
    long_mult(a_begin, a_end, b_begin, b_end, c);
    return;
  }
  values_type lower;
  values_type middle;
  values_type upper;
  const auto a_half = (a_size < half_max ? a_end : a_begin + half_max);
  const auto b_half = (b_size < half_max ? b_end : b_begin + half_max);

  // Reuse lower and upper to calculate middle.
  lower.insert(lower.begin(), a_begin, a_half);
  middle.insert(middle.begin(), a_half, a_end);
  base_op(lower, middle, &lower, add_op);
  middle.clear();
  upper.insert(upper.begin(), b_begin, b_half);
  middle.insert(middle.begin(), b_half, b_end);
  base_op(upper, middle, &upper, add_op);

  // Karatsuba recusion.
  karatsuba_mult(fallback_threshold, lower.cbegin(), lower.cend(),
                 upper.cbegin(), upper.cend(), &middle);
  karatsuba_mult(fallback_threshold, a_begin, a_half, b_begin, b_half, &lower);
  karatsuba_mult(fallback_threshold, a_half, a_end, b_half, b_end, &upper);

  // Build the solution.
  //   `upper << (2 * half_max * kBitsInBase) +
  //    (middle - upper - lower) << (half_max * kBitsInBase) +
  //    lower`.
  // This is done in the following steps:
  //   `middle -= lower + upper`  // This will never underflow.
  //   `middle <<= half_max * kBitsInBase`
  //   `upper = upper << (2 * half_max * kBitsInBase) + lower`
  //   `*c = upper + middle`
  base_op(middle, upper, &middle, dec_op);
  base_op(middle, lower, &middle, dec_op);
  middle.insert(middle.begin(), half_max, 0);
  // lower is `2 * half_max` elements long.
  upper.insert(upper.begin(), lower.cbegin(), lower.cend());
  base_op(upper, middle, c, add_op);
  normalize(c);
}

number& number::karatsuba(const number& other,
                          const values_size_type fallback_threshold) {
  karatsuba_mult(std::max(values_size_type{1}, fallback_threshold),
                 values_.cbegin(), values_.cend(),
                 other.values_.cbegin(), other.values_.cend(), &values_);
  sign_ ^= other.sign_;
  return *this;
}

void number::shift(int pos) {
  int big_steps = pos / kBitsInBase;
  pos -= big_steps * kBitsInBase;
  if (pos < 0) {
    --big_steps;
    pos += kBitsInBase;
  }
  // Now, pos >= 0.
  if (big_steps > 0) {
    if (values_.size() <= static_cast<nbase>(big_steps)) {
      values_.clear();
    } else {
      values_.erase(values_.cbegin(), values_.cbegin() + big_steps);
    }
  } else {
    values_.insert(values_.begin(), -big_steps, 0);
  }
  if (pos > 0 && !values_.empty()) {
    for (values_size_type i = 0; i + 1 < values_.size(); ++i) {
      values_[i] = (values_[i] >> pos) |
                   (values_[i + 1] << (kBitsInBase - pos));
    }
    values_.back() >>= pos;
  }
  normalize();
}

number& number::operator>>=(int pos) {
  shift(pos);
  return *this;
}

number number::operator>>(int pos) const {
  number result(*this);
  return result >>= pos;
}

number& number::operator<<=(int pos) {
  shift(-pos);
  return *this;
}

number number::operator<<(int pos) const {
  number result(*this);
  return result <<= pos;
}

// static.
std::tuple<number, number, number> number::gcd(const number& x,
                                               const number& y) {
  if (x == zero) {
    return std::make_tuple(one, one, y);
  }
  if (y == zero) {
    return std::make_tuple(one, one, x);
  }
  number u(x), v(y), a(1), b(0), c(0), d(1);
  int g = std::min(u.countr_zero(), v.countr_zero());
  u >>= g;
  v >>= g;
  const number x_(u), y_(v);
  while (u != zero) {
    while (u.even()) {
      u >>= 1;
      if (a.even() && b.even()) {
        a >>= 1;
        b >>= 1;
      } else {
        a += y_;
        a >>= 1;
        b -= x_;
        b >>= 1;
      }
    }
    while (v.even()) {
      v >>= 1;
      if (c.even() && d.even()) {
        c >>= 1;
        d >>= 1;
      } else {
        c += y_;
        c >>= 1;
        d -= x_;
        d >>= 1;
      }
    }
    if (u >= v) {
      u -= v;
      a -= c;
      b -= d;
    } else {
      v -= u;
      c -= a;
      d -= b;
    }
  }
  return std::make_tuple(c, d, v << g);
}

// static.
number::nbase number::inverse_mod_base(const nbase a) {
  // If nbase is n bits long, then we have to calculate
  //     `a^(2^(n - 1) - 1) mod 2^n`.
  nbase result = a;
  // At the end of each iteration, `result == a^(2^(i - 1) - 1) mod 2^n`.
  // Before the loop, `result == a`, this is,
  //     `result == a^1 == a^(2 - 1) == a^(2^(2 - 1) - 1)`.
  // Or in another way, the constraint holds for `i == 2`, so the loop starts
  // with `i == 3`.
  for (int i = 3; i < kBitsInBase; ++i) {
    result *= result;
    result *= a;
  }
  return result;
}

// static.
// This method makes the assumtion that `0 ≤ x, y < m` and that
// `gcd(m, 2) == 1`.
number number::montgomery(const number& m, const nbase& inv_m, const number& x,
                          const number& y) {
  if (m.length() == 0) return zero;
  number a;
  for (values_size_type i = 0; i < m.values_.size(); ++i) {
    nbase u = (a.at(0) + x.at(i) * y.at(0)) * inv_m;
    a = (a + number(x.at(i)) * y + number(u) * m) >> kBitsInBase;
  }
  if (a >= m) {
    a -= m;
  }
  return a;
}



number& number::pow_mod(const number& power, const number& modulus) {
  if (*this == zero) {
    return *this;
  }
  if (modulus == zero) {
    return *this;
  }
  if (power == zero) {
    *this = one;
    return *this;
  }

  // 0. Make `0 <= *this < modulus`.
  number q(modulus);
  if (q < zero) {
    q.neg();
  }
  if (abs_cmp(this->values_, q.values_) >= 0) {
    *this %= q;
  }
  if (*this < zero) {
    *this = q - *this;
  }

  // 1. Let `q` and `j` be so that `modulus = q * 2^j`, where `q` is odd.
  int j = modulus.countr_zero();
  q >>= j;

  // 2. Compute `x_2 = this->mod_pow(power, 2^j)` using the binary method and modulo arithmetics `2^j`.
  number x_2;
  if (j > 0) {
    x_2 = one;
    number w1(*this);
    w1.mod_pow2(j);
    number w2(power);
    w2.mod_pow2(j - 1);
    for (int i = w2.bit_size(); i >= 0; --i) {
      x_2 *= x_2;
      x_2.mod_pow2(j);
      if (w2.bit(i)) {
        x_2 *= w1;
        x_2.mod_pow2(j);
      }
    }
  }

  // 3. Compute `x_1 = this->mod_pow(power, q)` using Montgomery with sliding windows.
  number x_1;
  if (q > one) {
    x_1 = *this;
    x_1 %= q;

    int mask_size = exponentiation_mask_size(power.bit_size());
    // Build the work that will be used.
    nbase mask = (~nbase{0}) >> (kBitsInBase - mask_size);
    number power_(power);
    std::vector<std::pair<int, nbase>> work;
    while (power_ != zero) {
      int p2 = 0;
      while (power_.even()) {
        ++p2;
        power_ >>= 1;
      }
      work.emplace_back(p2, power_.at(0) & mask);
      power_ >>= mask_size;
    }

    nbase inv_m = -inverse_mod_base(q.at(0));
    std::vector<number> windows;
    windows.emplace_back(montgomery(q, inv_m, x_1,
        (one << (2 * kBitsInBase * q.length())) % q));
    if (mask_size > 1) {
      const number w2 = montgomery(q, inv_m, windows[0], windows[0]);
      for (int i = 1, m = 1 << (mask_size - 1); i < m; ++i) {
        windows.emplace_back(montgomery(q, inv_m, w2, windows[i - 1]));
      }
    }
    x_1 = (one << (kBitsInBase * q.length())) % q;
    for (auto it = work.crbegin(); it != work.crend(); it++) {
      for (int i = 0; i < mask_size; ++i) {
        x_1 = montgomery(q, inv_m, x_1, x_1);
      }
      x_1 = montgomery(q, inv_m, x_1, windows[it->second >> 1]);
      for (int i = 0; i < it->first; ++i) {
        x_1 = montgomery(q, inv_m, x_1, x_1);
      }
    }

    // Inverse montgomery.
    x_1 = montgomery(q, inv_m, x_1, one);
  }

  // 4. Compute `q^-1 (mod 2^j)` and `y = (x_2 - x_1)*(q^-1) (mod 2^j)`.
  number q_inv;
  std::tie(q_inv, std::ignore, std::ignore) = gcd(q, one << j);
  number y = ((x_2 - x_1)*q_inv).mod_pow2(j);


  // 5. Compute `x = x_1 + q * y`, and return x.
  *this = x_1 + q * y;
  return *this;
}



}  // namespace bignum
