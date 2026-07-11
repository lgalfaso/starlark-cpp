a = set([0, 1, 3])
b = set([3, 0, 2])

assert_eq(a | b, set([0, 1, 3, 2]))

assert_fail('''set() | ()''')

