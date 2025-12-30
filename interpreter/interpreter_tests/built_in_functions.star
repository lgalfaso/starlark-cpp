assert_eq(abs(-1), 1)
assert_eq(any([True, False]), True)
assert_eq(all([True, False]), False)
assert_eq(bool(1), True)
assert_eq(bytes("abc"), b"abc")
assert_eq(chr(1), "\x01")
# TODO(lmirelmann): Add tests for the other built-in functions once they are available.
# dict
# dir
# enumerate
# fail
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

