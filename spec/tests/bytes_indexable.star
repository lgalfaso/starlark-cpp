assert_fail('''b'abc'[-4]''')
assert_eq(b'abc'[-3], b'a')
assert_eq(b'abc'[-2], b'b')
assert_eq(b'abc'[-1], b'c')
assert_eq(b'abc'[0], b'a')
assert_eq(b'abc'[1], b'b')
assert_eq(b'abc'[2], b'c')
assert_fail('''b'abc'[3]''')

