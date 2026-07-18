# No arguments.
assert_fail('''len()''')


# One argument.
assert_eq(len([]), 0)
assert_eq(len([None]), 1)
assert_eq(len([None, None]), 2)
assert_eq(len(()), 0)
assert_eq(len((None,)), 1)
assert_eq(len((None, None)), 2)
assert_eq(len(set()), 0)
assert_eq(len(set([None])), 1)
assert_eq(len(set([None, 1])), 2)
assert_eq(len({}), 0)
assert_eq(len({None: None}), 1)
assert_eq(len({None: None, 1: 1}), 2)
assert_eq(len(range(0)), 0)
assert_eq(len(range(1)), 1)
assert_eq(len(range(2)), 2)

assert_fail('''len(1)''')
assert_fail('''len(True)''')
assert_fail('''len(None)''')


# Two arguments.
assert_fail('''len([], None)''')


# Named argument.
assert_fail('''len(object = [])''')

