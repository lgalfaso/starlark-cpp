# No arguments.
assert_eq(str(set()), 'set()')


# One argument.
assert_eq(str(set([0, None])), 'set([0, None])')
assert_fail('''set(1)''')
assert_fail('''set(None)''')
assert_fail('''set(True)''')
assert_fail('''set('')''')


# Two arguments.
assert_fail('''set([], None)''')


# Named arguments.
assert_fail('''set(iterable = [])''')

