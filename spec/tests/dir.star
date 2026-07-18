# No arguments.
assert_fail('''dir()''')


# One argument.
assert_eq(dir([]), ['append', 'clear', 'extend', 'index', 'insert', 'pop', 'remove'])
assert_eq(dir(None), [])


# Two arguments.
assert_fail('''dir([], None)''')


# Named arguments.
assert_fail('''dir(object = [])''')

