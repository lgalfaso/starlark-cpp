assert_fail('''(0, 1, 2)[-4]''')
assert_eq((0, 1, 2)[-3], 0)
assert_eq((0, 1, 2)[-2], 1)
assert_eq((0, 1, 2)[-1], 2)
assert_eq((0, 1, 2)[0], 0)
assert_eq((0, 1, 2)[1], 1)
assert_eq((0, 1, 2)[2], 2)
assert_fail('''(0, 1, 2)[3]''')

