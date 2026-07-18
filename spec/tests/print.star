# No arguments.
assert_succeed('''print()''', print = '\n')


# One argument.
assert_succeed('''print(0)''', print = '0\n')
assert_succeed('''print('hello')''', print = 'hello\n')


# Two argument.
assert_succeed('''print(0, [])''', print = '0 []\n')


# Named arguments.
assert_succeed('''print(0, [], sep = ', ')''', print = '0, []\n')
assert_fail('''print(0, [], sep = None)''')  # `The parameter `sep` must be a string.
assert_fail('''print(end = '')''')  # The named argument `end` is not allowed.

