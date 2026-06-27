l = [0, 1, 2]
assert_eq(l[-3], 0)
assert_eq(l[-2], 1)
assert_eq(l[-1], 2)
assert_eq(l[0], 0)
assert_eq(l[1], 1)
assert_eq(l[2], 2)

assert_fail('''[0, 1, 2][-4]''')
assert_fail('''[0, 1, 2][3]''')
assert_fail('''[0, 1, 2][1 << 64]''')
assert_fail('''[0, 1, 2][True]''')
assert_fail('''[0, 1, 2][None]''')

