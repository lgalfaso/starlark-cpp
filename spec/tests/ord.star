# No arguments.
assert_fail('''ord()''')


# One argument.
assert_fail('''ord('')''')
assert_fail('''ord(b'')''')
assert_eq(ord('A'), 65)
assert_eq(ord('😃'), 128515)
assert_eq(ord(b'\xff'), 255)
assert_fail('''ord('ab')''')
assert_fail('''ord(b'ab')''')
assert_fail('''ord([])''')
assert_fail('''ord(None)''')
assert_fail('''ord(True)''')
assert_fail('''ord(1)''')


# Two arguments.
assert_fail('''ord('a', None)''')


# Named arguments.
assert_fail('''ord(character = 'a')''')

