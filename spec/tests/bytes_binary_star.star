assert_eq(b'abc' * 2, b'abcabc')
assert_eq(b'abc' * 3, b'abcabcabc')
assert_eq(b'abc' * -2, b'')
assert_eq(b'abc' * -1, b'')
assert_eq(b'abc' * 0, b'')
assert_eq(b'' * 2, b'')
assert_eq(b'' * (1 << 64), b'')

assert_eq(2 * b'abc', b'abcabc')
assert_eq(3 * b'abc', b'abcabcabc')
assert_eq(-2 * b'abc', b'')
assert_eq(-1 * b'abc', b'')
assert_eq(0 * b'abc', b'')
assert_eq(2 * b'', b'')
assert_eq((1 << 64) * b'', b'')

assert_fail('''b'abc' * ()''')
assert_fail('''b'abc' * (1 << 64)''')

