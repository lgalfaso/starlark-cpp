
assert_eq(1 + 2 * 3 + 4, 11)
assert_eq((1 + 2) * (3 + 4), 21)

a, c, e = 1, 3, 5
b, d = True, False

assert_eq(a if b else c if d else e, 1)
assert_eq(a if b else (c if d else e), 1)
assert_eq((a if b else c) if d else e, 5)

assert_eq(type(lambda: a if d else c), "function")
assert_eq(type(lambda: (a if d else c)), "function")
assert_eq(type((lambda: a) if d else c), "int")

assert_eq(a if b else lambda: c if d else e, 1)
assert_eq(a if b else lambda: (c if d else e), 1)
assert_eq(type(a if d else lambda: c if d else e), "function")
assert_eq(type(a if d else lambda: (c if d else e)), "function")
assert_eq(type(a if d else (lambda: c if d else e)), "function")
assert_eq(a if d else (lambda: c) if d else e, 5)
assert_eq((a if b else lambda: c) if b else e, 1)

