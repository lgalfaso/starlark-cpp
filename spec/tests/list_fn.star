# No arguments.
assert_eq(list(), [])


# One argument.
a = [1, 2, 3]
b = list(a)
assert_eq(a, b)
a.append(4)
assert_eq(b, [1, 2, 3])
assert_eq(list(()), [])
assert_eq(list((1,)), [1])
assert_eq(list((1, 2)), [1, 2])
assert_eq(list(range(10)), [0, 1, 2, 3, 4, 5, 6, 7, 8, 9])

assert_fail('''list(1)''')
assert_fail('''list(None)''')
assert_fail('''list(True)''')


# Two arguments.
assert_fail('''list([], None)''')


# Named arguments.
assert_fail('''list(object = [])''')

