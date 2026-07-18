# No arguments.
assert_eq(str(), '')


# One argument.
assert_eq(str('abc'), 'abc')
assert_eq(str(b'abc'), 'b"abc"')
assert_eq(str(None), 'None')
assert_eq(str(1), '1')


# Two arguments.
assert_fail('''str('', None)''')


# Named arguments.
assert_eq(str(object = []), '[]')
assert_fail('''str([], object = [])''')
assert_fail('''str(encoding = 'utf-8')''')

