# No arguments.
assert_fail('''hasattr()''')


# One argument.
assert_fail('''hasattr([])''')


# Two arguments.
assert_true(hasattr([], "append"))
assert_false(hasattr([], "appen"))
assert_fail('''hasattr([], True)''')
assert_fail('''hasattr([], None)''')
assert_fail('''hasattr([], 1)''')


# Three arguments.
assert_fail('''hasattr([], "count", None)''')


# Named arguments.
assert_fail('''hasattr(object = [], name = "append")''')
assert_fail('''hasattr([], name = "append")''')

