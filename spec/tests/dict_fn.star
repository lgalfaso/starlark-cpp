# No arguments.
assert_eq(dict(), {})


# One argument.
a = {None: None}
b = dict(a)  # This must be a copy.
assert_eq(a, b)
a[1] = None
assert_eq(b, {None: None})

assert_eq(dict([(1, 'one'), (2, 'two')]), {1: 'one', 2: 'two'})
assert_eq(dict(one = 1, two = 2), {'one': 1, 'two': 2})

assert_fail('''dict(None)''')
assert_fail('''dict(True)''')
assert_fail('''dict(1)''')
assert_fail('''dict([(set(), 1)])''')  # Trying to set a non-hashable as key.
assert_fail('''dict([()])''')  # Too few elements in the tuple.
assert_fail('''dict([(1,)])''')  # Too few elements in the tuple.
assert_fail('''dict([(1, 1, 1)])''')  # Too many elements in the tuple.


# Two arguments.
assert_fail('''dict({}, {})''')

