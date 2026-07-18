# No arguments.
assert_fail('''range()''')


# One argument.
assert_eq(list(range(-1)), [])
assert_eq(list(range(0)), [])
assert_eq(list(range(1)), [0])
assert_eq(list(range(2)), [0, 1])
assert_eq(list(range(3)), [0, 1, 2])
assert_fail('''range([])''')
assert_fail('''range(True)''')
assert_fail('''range(None)''')


# Two arguments.
assert_eq(list(range(-1, 5)), [-1, 0, 1, 2, 3, 4])
assert_eq(list(range(5, 5)), [])
assert_eq(list(range(6, 5)), [])
assert_fail('''range(0, [])''')
assert_fail('''range(0, True)''')
assert_fail('''range(0, None)''')
assert_fail('''range([], 0)''')
assert_fail('''range(True, 0)''')
assert_fail('''range(None, 0)''')


# Three arguments.
assert_eq(list(range(1, 10, -1)), [])
assert_eq(list(range(1, -4, -1)), [1, 0, -1, -2, -3])
assert_eq(list(range(1, 10, 2)), [1, 3, 5, 7, 9])
assert_eq(list(range(1, -12, -3)), [1, -2, -5, -8, -11])
assert_fail('''range(0, 0, [])''')
assert_fail('''range(0, 0, True)''')
assert_fail('''range(0, 0, None)''')
assert_fail('''range(0, [], 1)''')
assert_fail('''range(0, True, 1)''')
assert_fail('''range(0, None, 1)''')
assert_fail('''range(0, 10, [])''')
assert_fail('''range(0, 10, True)''')
assert_fail('''range(0, 10, None)''')
assert_fail('''range(0, 10, 0)''')


# Four arguments.
assert_fail('''range(0, 10, 3, 2)''')


# Named arguments.
assert_fail('''range(end = 10)''')
assert_fail('''range(0, end = 10)''')
assert_fail('''range(start = 0, end = 10)''')
assert_fail('''range(0, 10, step = 1)''')
assert_fail('''range(0, end = 10, step = 1)''')
assert_fail('''range(start = 0, end = 10, step = 1)''')

