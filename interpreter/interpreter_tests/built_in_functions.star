assert_eq(abs(-1), 1)
assert_eq(any([True, False]), True)
assert_eq(all([True, False]), False)
assert_eq(bool(1), True)
assert_eq(bytes("abc"), b"abc")
assert_eq(chr(1), "\x01")
assert_eq(dict(), {})
assert_eq(dict({}), {})
assert_eq(dict({'a': 1}), {'a': 1})
assert_eq(dict({'a': 1}, b = 2), {'a': 1, 'b': 2})
assert_eq(dict([('a', 1), ('b', 2)]), {'a': 1, 'b': 2})
# TODO(lmirelmann): dir
assert_eq(enumerate(['a', 'b', 'c']), [(0, 'a'), (1, 'b'), (2, 'c')])
assert_eq(enumerate(['a', 'b', 'c'], start = 100), [(100, 'a'), (101, 'b'), (102, 'c')])
assert_fail("fail('Some message')", error_message = "Error: Some message")
assert_fail("float('2e308')", error_message = "OverflowError: floating-point number too large")
assert_eq(float(0.0), 0.0)
assert_eq(float(1), 1.0)
assert_eq(float("Inf"), 1e308*10)
assert_eq(float("-Inf"), -1e308*10)
# TODO(lmirelmann): getattr
# TODO(lmirelmann): hasattr
assert_eq(hash("abc"), 6041520446639342335)
assert_eq(int("0123", 10), 123)
assert_eq(int("0123"), 123)
assert_fail('int("0123", 0)', error_message = "ValueError: invalid literal for int() with base 0: '0123'")
assert_eq(len([]), 0)
assert_eq(list((1, 2)), [1, 2])
# TODO(lmirelmann): max
# TODO(lmirelmann): min
assert_eq(ord(b'\xff'), 255)
assert_eq(ord("😃"), 128515)
# TODO(lmirelmann): print
# TODO(lmirelmann): range
# TODO(lmirelmann): repr
# TODO(lmirelmann): reversed
# TODO(lmirelmann): set
# TODO(lmirelmann): sorted
# TODO(lmirelmann): str
# TODO(lmirelmann): tuple
# TODO(lmirelmann): type
# TODO(lmirelmann): zip

