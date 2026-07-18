# No arguments.
assert_fail('''any()''')


# One argument.
assert_false(any([]))
assert_false(any([False]))
assert_true(any([True]))
assert_false(any([False, False]))
assert_true(any([False, True]))
assert_true(any([True, False]))
assert_true(any([True, True]))

assert_fail('''any(1)''')
assert_fail('''any(None)''')
assert_fail('''any(True)''')


# Two arguments.
assert_fail('''any(False, False)''')


# Named arguments.
assert_fail('''any(iterable = [])''')

