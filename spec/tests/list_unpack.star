[a, b] = [1, 2]
assert_eq(a, 1)
assert_eq(b, 2)

assert_fail('''[a, b] = [1, 2, 3]''')
assert_fail('''[a, b] = [1]''')

