# No arguments.
assert_fail('''hash()''')


# One argument.
assert_succeed('''hash('')''')
assert_succeed('''hash(b'')''')
assert_true(hash('') == hash(''))

assert_fail('''hash(True)''')
assert_fail('''hash(None)''')
assert_fail('''hash(1)''')


# Two arguments.
assert_fail('''hash('', '')''')


# Named arguments.
assert_fail('''hash(object = '')''')

