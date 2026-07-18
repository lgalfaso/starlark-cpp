assert_eq(() * 2, ())
assert_eq(() * (1 << 64), ())
assert_eq((0, 1) * 2, (0, 1, 0, 1))
assert_eq((0, 1) * 3, (0, 1, 0, 1, 0, 1))
assert_eq((0, 1) * 0, ())
assert_eq((0, 1) * -1, ())
assert_eq((0, 1) * -2, ())

assert_eq(2 * (), ())
assert_eq((1 << 64) * (), ())
assert_eq(2 * (0, 1), (0, 1, 0, 1))
assert_eq(3 * (0, 1), (0, 1, 0, 1, 0, 1))
assert_eq(0 * (0, 1), ())
assert_eq(-1 * (0, 1), ())
assert_eq(-2 * (0, 1), ())


assert_fail('''() * None''')
assert_fail('''() * True''')

