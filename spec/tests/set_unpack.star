
[a, b] = set([0, 1])
assert_eq(a, 0)
assert_eq(b, 1)

assert_fail('''[a, b] = set()''')
assert_fail('''[a, b] = set([0])''')
assert_fail('''[a, b] = set([0, 1, 2])''')

