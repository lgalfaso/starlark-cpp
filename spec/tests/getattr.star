# No arguments.
assert_fail('''getattr()''')


# One argument.
assert_fail('''getattr(None)''')


# Two arguments.
a = []
m = getattr(a, "append")
m(True)
assert_eq(a, [True])

assert_fail('''getattr([], "unknown")''')
assert_eq(getattr([], "unknown", a), a)

assert_fail('''getattr([], None)''')
assert_fail('''getattr([], True)''')
assert_fail('''getattr([], 1)''')


# Four arguments.
assert_fail('''getattr([], "append", [], None)''')


# Named arguments.
assert_fail('''getattr(object = [], name = "append")''')
assert_fail('''getattr([], name = "append")''')
assert_fail('''getattr([], "append", default = [])''')

