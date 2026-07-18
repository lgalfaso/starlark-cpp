# No arguments.
assert_fail('''repr()''')


# One argument.
assert_eq(repr('abc'), '"abc"')
assert_eq(repr(None), 'None')


# Two arguments.
assert_fail('''repr(None, None)''')


# Named arguments.
assert_fail('''repr(object = None)''')

