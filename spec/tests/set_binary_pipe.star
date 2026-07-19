a = set([0, 1, 3])
b = set([3, 0, 2])

assert_eq(a | b, set([0, 1, 3, 2]))

assert_fail('''set() | ()''')

assert_eq(str(set([1]) | set([1.0])), 'set([1])')

