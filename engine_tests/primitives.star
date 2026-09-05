a = [0]
b = 1
c = 0x1234567890abcdefabcdef
d = None
e = {'a': 2}
f = 1.25
g = "abc"
h = b"def"
i = (1, 2, 3, 4)

assert_eq(a, [0])
assert_eq(b, 1)
assert_eq(c, 0x1234567890abcdefabcdef)
assert_eq(d, None)
assert_eq(e, {'a': 2})
assert_eq(f, 1.25)
assert_eq(g, "abc")
assert_eq(h, b"def")
assert_eq(i, (1, 2, 3, 4))
