assert_eq(0 and None, 0)
assert_eq(0 and 1, 0)
assert_eq(1 and None, None)
assert_eq(1 and 2, 2)
assert_eq(0 or None, None)
assert_eq(0 or 1, 1)
assert_eq(1 or None, 1)
assert_eq(1 or 2, 1)

