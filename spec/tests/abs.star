# No arguments.
assert_fail('''abs()''')

# One argument.
inf = 1e308 * 10
nan = inf/inf

assert_eq(abs(-1), 1)
assert_eq(abs(0), 0)
assert_eq(abs(1), 1)
assert_eq(abs(9223372036854775807), 9223372036854775807)
assert_eq(abs(-9223372036854775808), 9223372036854775808)
assert_eq(abs(-(1 << 64)), 18446744073709551616)
assert_eq(abs(1 << 64), 18446744073709551616)

assert_eq(abs(-1.0), 1.0)
assert_eq(abs(-0.0), 0.0)
assert_eq(abs(0.0), 0.0)
assert_eq(abs(1.0), 1.0)

assert_eq(abs(-1.7976931348623157e+308), 1.7976931348623157e+308)
assert_eq(abs(1.7976931348623157e+308), 1.7976931348623157e+308)
assert_eq(abs(-2.2250738585072014e-308), 2.2250738585072014e-308)
assert_eq(abs(2.2250738585072014e-308), 2.2250738585072014e-308)

assert_eq(abs(-nan), nan)
assert_eq(abs(nan), nan)
assert_eq(abs(-inf), inf)
assert_eq(abs(inf), inf)

assert_fail('''abs([])''')
assert_fail('''abs(False)''')
assert_fail('''abs(None)''')


# Two arguments.
assert_fail('''abs(0, 0)''')


# Named arguments.
assert_fail('''abs(number = 0)''')

