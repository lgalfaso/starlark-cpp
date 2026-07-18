# No arguments.
assert_eq(bytes(), b'')


# One argument.
assert_eq(bytes(''), b'')
assert_eq(bytes('abc'), b'abc')
assert_eq(bytes('∆ß¬ƒª™£'), b'\xe2\x88\x86\xc3\x9f\xc2\xac\xc6\x92\xc2\xaa\xe2\x84\xa2\xc2\xa3')

assert_eq(bytes(b''), b'')
assert_eq(bytes(b'\xff\xfa'), b'\xff\xfa')

assert_eq(bytes([]), b'')
assert_eq(bytes([0, 1, 254, 255]), b'\x00\x01\xfe\xff')
assert_fail('''bytes([-1])''')
assert_fail('''bytes([256])''')
assert_fail('''bytes([None])''')

assert_eq(bytes((0, 1, 2)), b'\x00\x01\x02')

assert_fail('''bytes(None)''')
assert_fail('''bytes(False)''')
assert_fail('''bytes(1)''')


# Two arguments.
assert_fail('''bytes([], None)''')


# Named arguments.
assert_eq(bytes(source = []), b'')
assert_fail('''bytes([], source = [])''')
