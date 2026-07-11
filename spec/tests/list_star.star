l1 = [0, 1, 2]
l2 = []
assert_eq(l1 * 2, [0, 1, 2, 0, 1, 2])
assert_eq(l1 * 3, [0, 1, 2, 0, 1, 2, 0, 1, 2])
assert_eq(l2 * 3, [])
assert_eq(l1 * 0, [])
assert_eq(l1 * -1, [])
assert_eq(l1 * -2, [])
assert_eq(l2 * (1 << 64), [])

assert_eq(2 * l1, [0, 1, 2, 0, 1, 2])
assert_eq(3 * l1, [0, 1, 2, 0, 1, 2, 0, 1, 2])
assert_eq(3 * l2, [])
assert_eq(0 * l1, [])
assert_eq(-1 * l1, [])
assert_eq(-2 * l1, [])
assert_eq((1 << 64) * l2, [])

assert_fail('''None * []''')
assert_fail('''False * []''')

