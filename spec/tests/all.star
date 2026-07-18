# No arguments.
assert_fail('''all()''')


# One argument.
assert_true(all([]))
assert_true(all(set()))
assert_false(all([0]))
assert_true(all([1]))
assert_true(all([1, True]))
assert_false(all([0, True]))
assert_false(all([1, False]))
assert_false(all([0, False]))

assert_fail('''all(1)''')
assert_fail('''all(None)''')
assert_fail('''all(True)''')


# Two arguments.
assert_fail('''all([], [])''')


# Named arguments.
assert_fail('''all(iterable = [])''')

