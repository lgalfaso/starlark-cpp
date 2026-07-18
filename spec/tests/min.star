# No arguments.
assert_fail('''min()''')


# One argument.
assert_fail('''min([])''')
assert_eq(min([1]), 1)
assert_eq(min([1, 2]), 1)
assert_eq(min([1, 0, 2]), 0)
assert_fail('''min([1, True])''')
assert_fail('''min(1)''')
assert_fail('''min(None)''')
assert_fail('''min(True)''')


# Two or more arguments.
assert_eq(min(1, 2), 1)
assert_eq(min(1, 2, 3), 1)
assert_eq(min(1, 2, 0, 3), 0)
assert_fail('''min(1, True)''')


# Named argument.
assert_eq(min(['one', 'two', 'three', 'four'], key = len), 'one')
assert_eq(min(['one', 'two', 'three', 'four'], key = None), 'four')
assert_fail('''min([1, 'two', 'three', 'four'], key = len)''')
assert_fail('''min(['one', 2, 'three', 'four'], key = len)''')
assert_fail('''min([[], []], key = set)''')

assert_eq(min('one', 'two', 'three', 'four', key = len), 'one')
assert_eq(min('one', 'two', 'three', 'four', key = None), 'four')
assert_fail('''min(1, 'two', 'three', 'four', key = len)''')
assert_fail('''min('one', 2, 'three', 'four', key = len)''')
assert_fail('''min([], [], key = set)''')
assert_fail('''min(iterable = [1])''')

