# No arguments.
assert_fail('''b'abc'.replace()''')

# One argument.
assert_fail('''b'abc'.replace(b'x')''')

# Two arguments.
assert_eq(b''.replace(b'', b''), b'')
assert_eq(b''.replace(b'', b'x'), b'x')
assert_eq(b'a'.replace(b'', b'x'), b'xax')
assert_eq(b'ab'.replace(b'', b'x'), b'xaxbx')

assert_eq(b''.replace(b'y', b''), b'')
assert_eq(b''.replace(b'y', b'x'), b'')
assert_eq(b'a'.replace(b'y', b'x'), b'a')
assert_eq(b'ayb'.replace(b'y', b'x'), b'axb')
assert_eq(b'ayyb'.replace(b'y', b'x'), b'axxb')
assert_eq(b'ayxyb'.replace(b'y', b'x'), b'axxxb')
assert_eq(b'ayxyb'.replace(b'y', b'yy'), b'ayyxyyb')

assert_fail('''b'abc'.replace(97, b'')''')
assert_fail('''b'abc'.replace(b'', 97)''')
assert_fail('''b'abc'.replace(False, b'')''')
assert_fail('''b'abc'.replace(b'', False)''')
assert_fail('''b'abc'.replace(None, b'')''')
assert_fail('''b'abc'.replace(b'', None)''')


# Three arguments.
assert_eq(b''.replace(b'', b'', -1), b'')
assert_eq(b''.replace(b'', b'', 0), b'')
assert_eq(b''.replace(b'', b'', 1), b'')
assert_eq(b''.replace(b'', b'x', -1), b'x')
assert_eq(b''.replace(b'', b'x', 0), b'')
assert_eq(b''.replace(b'', b'x', 1), b'x')
assert_eq(b'a'.replace(b'', b'x', -1), b'xax')
assert_eq(b'a'.replace(b'', b'x', 0), b'a')
assert_eq(b'a'.replace(b'', b'x', 1), b'xa')
assert_eq(b'ab'.replace(b'', b'x', -1), b'xaxbx')
assert_eq(b'ab'.replace(b'', b'x', 0), b'ab')
assert_eq(b'ab'.replace(b'', b'x', 1), b'xab')
assert_eq(b'ab'.replace(b'', b'x', 2), b'xaxb')

assert_eq(b''.replace(b'y', b'', -1), b'')
assert_eq(b''.replace(b'y', b'', 0), b'')
assert_eq(b''.replace(b'y', b'', 1), b'')
assert_eq(b''.replace(b'y', b'x', -1), b'')
assert_eq(b''.replace(b'y', b'x', 0), b'')
assert_eq(b''.replace(b'y', b'x', 1), b'')
assert_eq(b'a'.replace(b'y', b'x', -1), b'a')
assert_eq(b'a'.replace(b'y', b'x', 0), b'a')
assert_eq(b'a'.replace(b'y', b'x', 1), b'a')
assert_eq(b'ab'.replace(b'y', b'x', -1), b'ab')
assert_eq(b'ab'.replace(b'y', b'x', 0), b'ab')
assert_eq(b'ab'.replace(b'y', b'x', 1), b'ab')
assert_eq(b'ayb'.replace(b'y', b'x', -1), b'axb')
assert_eq(b'ayb'.replace(b'y', b'x', 0), b'ayb')
assert_eq(b'ayb'.replace(b'y', b'x', 1), b'axb')
assert_eq(b'ayyb'.replace(b'y', b'x', -1), b'axxb')
assert_eq(b'ayyb'.replace(b'y', b'x', 0), b'ayyb')
assert_eq(b'ayyb'.replace(b'y', b'x', 1), b'axyb')
assert_eq(b'ayxyb'.replace(b'y', b'x', -1), b'axxxb')
assert_eq(b'ayxyb'.replace(b'y', b'x', 0), b'ayxyb')
assert_eq(b'ayxyb'.replace(b'y', b'x', 1), b'axxyb')
assert_eq(b'ayxyb'.replace(b'y', b'yy', -1), b'ayyxyyb')
assert_eq(b'ayxyb'.replace(b'y', b'yy', 0), b'ayxyb')
assert_eq(b'ayxyb'.replace(b'y', b'yy', 1), b'ayyxyb')

assert_fail('''b'abbbc'.replace(b'b', b'x', None)''')
assert_fail('''b'abbbc'.replace(b'b', b'x', False)''')

# Four arguments.
assert_fail('''b'abc'.replace(b'', b'', 1, None)''')

# Named arguments.
assert_fail('''b'abc'.replace(b'', old = b'')''')
assert_fail('''b'abc'.replace(b'', new = b'')''')
assert_eq(b'abbbc'.replace(b'b', b'x', count = 2), b'axxbc')
assert_fail('''b'abc'.replace(b'', b'', 1, count = 1)''')

