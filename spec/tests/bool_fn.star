# No arguments.
assert_false(bool())


# One argument.
assert_false(bool(None))
assert_false(bool([]))
assert_true(bool([1]))
assert_false(bool(0))
assert_true(bool(1))
assert_false(bool(''))
assert_true(bool('x'))


# Two arguments.
assert_fail('''bool(0, None)''')


# Named arguments.
assert_fail('''bool(object = None)''')

