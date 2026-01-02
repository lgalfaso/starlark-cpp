// Copyright 2025 Lucas Mirelmann

#ifndef RUNTIME_STARLARK_NUMERIC_HPP_
#define RUNTIME_STARLARK_NUMERIC_HPP_

#include "bigint/number.hpp"
#include "runtime/starlark_object.hpp"

#pragma GCC visibility push(default)

namespace starlark {
namespace runtime {

bool equals_fb(double lhs, const starlark::bigint::number& rhs);
bool equals_fi(double lhs, int64_t rhs);
bool equals_ib(int64_t lhs, const starlark::bigint::number& rhs);
int cmp_fb(double lhs, const starlark::bigint::number& rhs);
int cmp_fi(double lhs, int64_t rhs);
int cmp_ib(int64_t lhs, const starlark::bigint::number& rhs);
starlark::bigint::number from_int64(int64_t value);
double to_double(const starlark::bigint::number& value);
double starlark_fmod(double a, double b);
starlark::bigint::number starlark_div(const starlark::bigint::number& a, const starlark::bigint::number& b);
starlark::bigint::number starlark_mod(const starlark::bigint::number& a, const starlark::bigint::number& b);
int64_t starlark_div(int64_t a, int64_t b);
int64_t starlark_mod(int64_t a, int64_t b);

starlark_obj* create_integer(std::int64_t value, google::protobuf::Arena& arena);
starlark_obj* create_integer(starlark::bigint::number&& value, google::protobuf::Arena& arena);
starlark_obj* create_integer_from_float(double value, google::protobuf::Arena& arena);
starlark_obj* create_float(double value, google::protobuf::Arena& arena);

}  // namespace runtime
}  // namespace starlark

#pragma GCC visibility pop

#endif  // RUNTIME_STARLARK_NUMERIC_HPP_

