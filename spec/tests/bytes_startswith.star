# No arguments.

assert_fail('''b'abc'.startswith()''')


# One argument.

assert_true(b''.startswith(b''))
assert_false(b''.startswith(b'a'))
assert_true(b'abc'.startswith(b''))
assert_true(b'abc'.startswith(b'a'))
assert_true(b'abc'.startswith(b'abc'))
assert_false(b'abc'.startswith(b'c'))
assert_false(b'abc'.startswith(b'abcc'))

assert_false(b'abc'.startswith(()))
assert_true(b''.startswith((b'',)))
assert_false(b''.startswith((b'a',)))
assert_true(b'abc'.startswith((b'',)))
assert_true(b'abc'.startswith((b'a',)))
assert_true(b'abc'.startswith((b'abc',)))
assert_false(b'abc'.startswith((b'c',)))
assert_false(b'abc'.startswith((b'abcc',)))

assert_true(b''.startswith((b'', b'a')))
assert_true(b''.startswith((b'a', b'')))
assert_true(b'abc'.startswith((b'', b'a')))
assert_true(b'abc'.startswith((b'a', b'abc')))
assert_true(b'abc'.startswith((b'abc', b'c')))
assert_false(b'abc'.startswith((b'c', b'abcc')))
assert_true(b'abc'.startswith((b'abcc', b'')))

assert_fail('''b'abc'.startswith(None)''')
assert_fail('''b'abc'.startswith('')''')
assert_fail('''b'abc'.startswith(97)''')
assert_fail('''b'abc'.startswith((None,))''')
assert_fail('''b'abc'.startswith(('',))''')
assert_fail('''b'abc'.startswith((97,))''')
assert_fail('''b'abc'.startswith((b'', 97))''')


# Two arguments.

assert_true(b''.startswith(b'', 0))
assert_false(b''.startswith(b'', 1))
assert_false(b''.startswith(b'a', 0))
assert_false(b''.startswith(b'a', 1))
assert_true(b'abc'.startswith(b'a', 0))
assert_false(b'abc'.startswith(b'a', 1))
assert_false(b'abc'.startswith(b'a', 2))
assert_false(b'abc'.startswith(b'a', 3))
assert_false(b'abc'.startswith(b'a', 4))
assert_true(b'abc'.startswith(b'abc', -4))
assert_true(b'abc'.startswith(b'abc', -3))
assert_false(b'abc'.startswith(b'abc', -2))
assert_false(b'abc'.startswith(b'abc', -1))
assert_true(b'abc'.startswith(b'abc', 0))
assert_false(b'abc'.startswith(b'abc', 1))
assert_false(b'abc'.startswith(b'abc', 2))
assert_false(b'abc'.startswith(b'abc', 3))
assert_false(b'abc'.startswith(b'abc', 4))
assert_false(b'abc'.startswith(b'abc', 5))

assert_true(b'abc'.startswith(b'', 0))
assert_true(b'abc'.startswith(b'', 1))
assert_true(b'abc'.startswith(b'', 2))
assert_true(b'abc'.startswith(b'', 3))
assert_false(b'abc'.startswith(b'', 4))
assert_false(b'abc'.startswith(b'c', 0))
assert_false(b'abc'.startswith(b'c', 1))
assert_true(b'abc'.startswith(b'c', 2))
assert_false(b'abc'.startswith(b'c', 3))
assert_false(b'abc'.startswith(b'abcc', -1))
assert_false(b'abc'.startswith(b'abcc', 0))
assert_false(b'abc'.startswith(b'abcc', 3))

assert_fail('''b'abc'.startswith(b'', True)''')
assert_true(b'abc'.startswith(b'', None))


# Three arguments.

assert_true(b''.startswith(b'', 0, -1))
assert_true(b''.startswith(b'', 0, 0))
assert_false(b''.startswith(b'', 1, 0))
assert_false(b''.startswith(b'a', 0, 1))
assert_false(b''.startswith(b'a', 1, 1))
assert_true(b'abc'.startswith(b'a', 0, 3))
assert_true(b'abc'.startswith(b'a', 0, 2))
assert_false(b'abc'.startswith(b'a', 1, 3))
assert_false(b'abc'.startswith(b'a', 1, 2))
assert_false(b'abc'.startswith(b'c', 0, 3))
assert_true(b'abc'.startswith(b'c', 2, 3))
assert_false(b'abc'.startswith(b'c', 3, 3))
assert_false(b'abc'.startswith(b'c', 1, 2))
assert_true(b'abc'.startswith(b'b', 1, 2))
assert_false(b'abc'.startswith(b'abc', 0, -1))
assert_true(b'abc'.startswith(b'ab', 0, -1))
assert_false(b'abc'.startswith(b'abc', 0, 2))
assert_true(b'abc'.startswith(b'abc', 0, 3))
assert_true(b'abc'.startswith(b'abc', 0, 4))
assert_false(b'abc'.startswith(b'abc', 1, 3))
assert_false(b'abc'.startswith(b'abc', -1, 4))
assert_false(b'abc'.startswith(b'abc', -2, 10))
assert_true(b'abc'.startswith(b'', 0, 3))
assert_true(b'abc'.startswith(b'', 2, 3))
assert_false(b'abc'.startswith(b'', 2, 1))
assert_true(b'abc'.startswith(b'', 3, 3))
assert_false(b'abc'.startswith(b'', 4, 3))
assert_true(b'abc'.startswith(b'a', 0, 3))
assert_true(b'abc'.startswith(b'a', 0, 1))
assert_false(b'abc'.startswith(b'a', 3, 3))
assert_false(b'abc'.startswith(b'aabc', -1, 2))
assert_false(b'abc'.startswith(b'aabc', 0, 4))
assert_false(b'abc'.startswith(b'aabc', 3, 3))

assert_fail('''b'abc'.startswith(b'', 0, True)''')
assert_true(b'abc'.startswith(b'', 0, None))


# Four arguments.

assert_fail('''b'abc'.startswith(b'', None, None, None)''')


# Named arguments.
assert_fail('''b'abc'.startswith(prefix = b'')''')
assert_fail('''b'abc'.startswith(b'', start = 0)''')
assert_fail('''b'abc'.startswith(b'', 0, end = 0)''')

