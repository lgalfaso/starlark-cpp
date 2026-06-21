// Copyright 2025 Lucas Mirelmann

#include "runtime/starlark_numeric.hpp"

#include <cmath>

#include <bit>
#include <limits>
#include <string>

#include "runtime/starlark_types.hpp"

using ::starlark::bigint::number;

namespace starlark {
namespace runtime {

bool equals_fi(double lhs, int64_t rhs) {
  if (!std::isfinite(lhs)) {
    return false;
  }
  if (rhs == 0) {
    return lhs == 0;
  }
  if (lhs == 0) {
    return false;
  }
  if ((lhs < 0) ^ (rhs < 0)) {
    return false;
  }
  if (lhs < 0) {
    lhs = -lhs;
  }
  uint64_t urhs = rhs < 0 ? static_cast<uint64_t>(-rhs) : rhs;
  int e;
  double norm = std::frexp(lhs, &e);
  double integral = std::ldexp(norm, std::numeric_limits<double>::digits);
  e -= std::numeric_limits<double>::digits;
  int64_t mantissa = static_cast<int64_t>(integral);
  {
     int countr = std::countr_zero<uint64_t>(mantissa);
     mantissa >>= countr;
     e += countr;
  }
  if (e < 0) {
    return false;
  }
  if (std::countr_zero(urhs) != e ||
      64 - std::countl_zero(urhs) != e + 64 - std::countl_zero<uint64_t>(mantissa)) {
    return false;
  }
  return mantissa == (urhs >> e);
}

bool equals_fb(double lhs, const number& rhs) {
  if (!std::isfinite(lhs)) {
    return false;
  }
  if (rhs == number::zero()) {
    return lhs == 0;
  }
  if (lhs == 0) {
    return false;
  }
  if ((lhs < 0) ^ (rhs.sign())) {
    return false;
  }
  if (lhs < 0) {
    lhs = -lhs;
  }
  int e;
  double norm = std::frexp(lhs, &e);
  double integral = std::ldexp(norm, std::numeric_limits<double>::digits);
  e -= std::numeric_limits<double>::digits;
  int64_t mantissa = static_cast<int64_t>(integral);
  {
     int countr = std::countr_zero<uint64_t>(mantissa);
     mantissa >>= countr;
     e += countr;
  }
  if (e < 0) {
    return false;
  }
  if (rhs.countr_zero() != e ||
      rhs.bit_size() != e + 64 - std::countl_zero<uint64_t>(mantissa)) {
    return false;
  }
  return mantissa == rhs.bits(e, 64);
}

bool equals_ib(int64_t lhs, const number& rhs) {
  if (rhs == number::zero()) {
    return lhs == 0;
  }
  if (lhs == 0) {
    return false;
  }
  if ((lhs < 0) ^ (rhs.sign())) {
    return false;
  }
  if (rhs.length() != 1) {
    return false;
  }
  if (lhs < 0) {
    // This only works if the representation is two's complement, as other
    // representations might do the wrong thing for `std::numeric_limits<int64_t>::min()`.
    lhs = -lhs;
  }
  return rhs.at(0) == lhs;
}

int cmp_fi(double lhs, int64_t rhs) {
  // Simple cases when `lhs` is infinite, there is a difference in sign or one of the inputs is zero.
  if (!std::isfinite(lhs)) {
    return lhs > 0 ? 1 : -1;
  }
  if (rhs == 0) {
    if (lhs != 0) {
      return lhs > 0 ? 1 : -1;
    }
    return 0;
  }
  if (lhs == 0) {
    return rhs < 0 ? 1 : -1;
  }

  if ((lhs < 0) ^ (rhs < 0)) {
    return rhs < 0 ? 1 : -1;
  }
  if (lhs < 0) {
    lhs = -lhs;
  }
  uint64_t urhs = rhs < 0 ? static_cast<uint64_t>(-rhs) : rhs;

  // Split `lhs` in a mantissa, an exponent for the integral part and a flag stating whether
  // there is a factional part after the integral part.
  int e;
  double norm = std::frexp(lhs, &e);
  double integral = std::ldexp(norm, std::numeric_limits<double>::digits);
  e -= std::numeric_limits<double>::digits;
  int64_t mantissa = static_cast<int64_t>(integral);
  {
    int countr = std::countr_zero<uint64_t>(mantissa);
    mantissa >>= countr;
    e += countr;
  }
  bool has_fraction = false;
  if (e < 0) {
    has_fraction = true;
    if (e < -63) {
      mantissa = 0;
      e = 0;
    } else {
      mantissa >>= (-e);
      if (mantissa == 0) {
        e = 0;
      } else {
        e = std::countr_zero<uint64_t>(mantissa);
        mantissa >>= e;
      }
    }
  }

  // Check if there is a difference in the bit size.
  int lhs_bit_size = e + 64 - std::countl_zero<uint64_t>(mantissa);
  int rhs_bit_size = 64 - std::countl_zero(urhs);
  if (rhs_bit_size != lhs_bit_size) {
    return (rhs_bit_size > lhs_bit_size) ^ (rhs < 0) ? -1 : 1;
  }

  // Check if there is a difference in the high bits.
  auto rhs_high_bits = (urhs >> e);
  if (mantissa != rhs_high_bits) {
    return (rhs_high_bits > mantissa) ^ (rhs < 0) ? -1 : 1;
  }

  // Check if there is a difference in the integral part after the high bits.
  if (std::countr_zero(urhs) != e) {
    return (rhs < 0) ? 1 : -1;
  }

  // Check if there is a difference in the fractional part.
  if (has_fraction) {
    return (rhs < 0) ? -1 : 1;
  }

  // The numbers are equal.
  return 0;
}

int cmp_fb(double lhs, const number& rhs) {
  // Simple cases when `lhs` is infinite, there is a difference in sign or one of the inputs is zero.
  if (!std::isfinite(lhs)) {
    return lhs > 0 ? 1 : -1;
  }
  if (rhs == number::zero()) {
    if (lhs != 0) {
      return lhs > 0 ? 1 : -1;
    }
    return 0;
  }
  if (lhs == 0) {
    return rhs.sign() ? 1 : -1;
  }

  if ((lhs < 0) ^ (rhs.sign())) {
    return rhs.sign() ? 1 : -1;
  }
  if (lhs < 0) {
    lhs = -lhs;
  }

  // Split `lhs` in a mantissa, an exponent for the integral part and a flag stating whether
  // there is a factional part after the integral part.
  int e;
  double norm = std::frexp(lhs, &e);
  double integral = std::ldexp(norm, std::numeric_limits<double>::digits);
  e -= std::numeric_limits<double>::digits;
  int64_t mantissa = static_cast<int64_t>(integral);
  {
    int countr = std::countr_zero<uint64_t>(mantissa);
    mantissa >>= countr;
    e += countr;
  }
  bool has_fraction = false;
  if (e < 0) {
    has_fraction = true;
    if (e < -63) {
      mantissa = 0;
      e = 0;
    } else {
      mantissa >>= (-e);
      if (mantissa == 0) {
        e = 0;
      } else {
        e = std::countr_zero<uint64_t>(mantissa);
        mantissa >>= e;
      }
    }
  }

  // Check if there is a difference in the bit size.
  int lhs_bit_size = e + 64 - std::countl_zero<uint64_t>(mantissa);
  int rhs_bit_size = rhs.bit_size();
  if (rhs_bit_size != lhs_bit_size) {
    return (rhs_bit_size > lhs_bit_size) ^ rhs.sign() ? -1 : 1;
  }

  // Check if there is a difference in the high bits.
  auto rhs_high_bits = rhs.bits(e, 64);
  if (mantissa != rhs_high_bits) {
    return (rhs_high_bits > mantissa) ^ rhs.sign() ? -1 : 1;
  }

  // Check if there is a difference in the integral part after the high bits.
  if (rhs.countr_zero() != e) {
    return rhs.sign() ? 1 : -1;
  }

  // Check if there is a difference in the fractional part.
  if (has_fraction) {
    return rhs.sign() ? -1 : 1;
  }

  // The numbers are equal.
  return 0;
}

int cmp_ib(int64_t lhs, const number& rhs) {
  if (rhs == number::zero()) {
      if (lhs != 0) {
        return lhs > 0 ? 1 : -1;
      }
      return 0;
  }
  if (lhs == 0) {
    return rhs.sign() ? 1 : -1;
  }
  if ((lhs < 0) ^ (rhs.sign())) {
    return rhs.sign() ? 1 : -1;
  }
  if (rhs.length() != 1) {
    return rhs.sign() ? 1 : -1;
  }
  if (lhs < 0) {
    // This only works if the representation is two's complement, as other
    // representations might do the wrong thing for `std::numeric_limits<int64_t>::min()`.
    lhs = -lhs;
  }
  auto ulhs = static_cast<uint64_t>(lhs);
  auto rhs_first_block = rhs.at(0);
  if (ulhs != rhs_first_block) {
    return (rhs_first_block > ulhs) ^ rhs.sign() ? -1 : 1;
  }
  return 0;
}

number from_int64(int64_t value) {
  number result(value);
  if (value < 0) {
    result -= number::one() << 64;
  }
  return result;
}

double to_double(const number& value) {
  int bit_size = value.bit_size();
  if (bit_size > 1024) {
    return value.sign() ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
  }
  if (bit_size <= 64) {
    double result = static_cast<double>(value.at(0));
    return value.sign() ? -result : result;
  }
  // We are taking two extra bits to make sure that the value is rounded up if the last bit is a one.
  auto digits = std::numeric_limits<double>::digits + 2;
  auto relevant_bits = value.bits(bit_size - digits, digits);
  // And if there is any `1` in the remaining bits, then add one more.
  if ((relevant_bits & 3) == 2 && value.countr_zero() + digits < bit_size) {
    relevant_bits++;
  }
  double relevant_digits = relevant_bits;
  double unsigned_result = std::ldexp(relevant_digits, bit_size - digits);
  return value.sign() ? -unsigned_result : unsigned_result;
}

double starlark_fmod(double a, double b) {
  double result = std::fmod(a, b);
  if (result == 0.0) {
    return copysign(result, b);
  }
  if ((result < 0) ^ (b < 0)) {
    result += b;
  }
  return result;
}

number starlark_div(const number& a, const number& b) {
  number d, r;
  std::tie(d, r) = number::div(a, b);
  if (b.sign() != r.sign() && r != number::zero()) {
    d -= number::one();
  }
  return d;
}

number starlark_mod(const number& a, const number& b) {
  number d, r;
  std::tie(d, r) = number::div(a, b);
  if (b.sign() != r.sign() && r != number::zero()) {
    r += b;
  }
  return r;
}

int64_t starlark_div(int64_t a, int64_t b) {
  auto result = a / b;
  if (a < 0 != b < 0 && result * b != a) {
    return result - 1;
  }
  return result;
}

int64_t starlark_mod(int64_t a, int64_t b) {
  auto result = a % b;
  if (a < 0 != b < 0 && result != 0) {
    return result + b;
  }
  return result;
}

std::string float_to_string(double value, bool uppercase) {
  if (std::isnan(value)) {
    return "nan";
  }
  if (std::isinf(value)) {
    return (value < 0) ? "-inf" : "inf";
  }
  if (value == 0.0) {
    return (std::signbit(value)) ? "-0.0" : "0.0";
  }

  // Enforce exact scientific notation thresholds.
  double abs_value = std::abs(value);
  bool use_scientific = (abs_value < 0.0001 || abs_value >= 10000000000000000.0);
  auto mode = use_scientific ? std::chars_format::scientific : std::chars_format::fixed;

  std::array<char, 64> buf;
  auto [ptr, ec] = std::to_chars(buf.data(), buf.data() + buf.size(), value, mode);
  if (ec != std::errc()) {
    return "";
  }
  std::string result(buf.data(), ptr);

  if (!use_scientific) {
    if (result.find('.') == std::string::npos) {
      result += ".0";
    }
  } else {
    if (uppercase) {
      if (size_t e_pos = result.find('e'); e_pos != std::string::npos) {
        result[e_pos] = 'E';
      }
    }
  }
  return result;
}

}  // namespace runtime
}  // namespace starlark


