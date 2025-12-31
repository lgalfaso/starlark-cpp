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
# float
# getattr
# hasattr
# hash
# int
assert_eq(len([]), 0)
assert_eq(list((1, 2)), [1, 2])
# max
# min
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

