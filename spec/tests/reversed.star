# No arguments.
assert_fail('''reversed()''')


# One argument.
assert_eq(reversed([]), [])
assert_eq(reversed((1, 2, 3)), [3, 2, 1])
assert_fail('''reversed(1)''')
assert_fail('''reversed(None)''')
assert_fail('''reversed(True)''')


# Two arguments.
assert_fail('''reversed(1, 2)''')


# Named arguments.
assert_fail('''reversed(object = [])''')

