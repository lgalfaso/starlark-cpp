# No arguments.
assert_eq(tuple(), ())


# One argument.
assert_eq(tuple([]), ())
assert_eq(tuple([None]), (None,))
assert_eq(tuple([None, True]), (None, True))
assert_fail('''tuple('')''')


# Two arguments.
assert_fail('''tuple([], None)''')


# Named argument.
assert_fail('''tuple(iterable = [])''')

