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
# TODO(lmirelmann): Add tests for the other built-in functions once they are available.
# dir
assert_eq(enumerate(['a', 'b', 'c']), [(0, 'a'), (1, 'b'), (2, 'c')])
assert_eq(enumerate(['a', 'b', 'c'], start = 100), [(100, 'a'), (101, 'b'), (102, 'c')])
assert_fail("fail('Some message')", error_message = "Error: Some message")
assert_fail("float('2e308')", error_message = "OverflowError: floating-point number too large")
assert_eq(float(0.0), 0.0)
assert_eq(float(1), 1.0)
assert_eq(float("Inf"), 1e308*10)
assert_eq(float("-Inf"), -1e308*10)
# getattr
# hasattr
assert_eq(hash("abc"), 6041520446639342335)
# int
assert_eq(len([]), 0)
assert_eq(list((1, 2)), [1, 2])
# max
# min
assert_eq(ord(b'\xff'), 255)
assert_eq(ord("😃"), 128515)
# ord
# print
# range
# repr
# reversed
# set
# sorted
# str
# tuple
# type
# zip

