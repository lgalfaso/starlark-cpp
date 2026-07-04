# No arguments.
assert_fail('''b''.join()''')

# One argument.
assert_fail('''b''.join(b'')''')
assert_eq(b'abc'.join(()), b'')
assert_fail('''b'abc'.join((1,))''')
assert_eq(b'abc'.join((b'xyz',)), b'xyz')
assert_fail('''b'abc'.join((False,))''')
assert_eq(b'abc'.join((b'xyz', b'qwe')), b'xyzabcqwe')
assert_eq(b'abc'.join((b'xyz', b'qwe', b'rty')), b'xyzabcqweabcrty')
assert_fail('''b''.join((), 0)''')
assert_fail('''b''.join(iterable = ())''')

