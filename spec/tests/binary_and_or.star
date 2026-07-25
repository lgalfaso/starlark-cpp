assert_eq(0 and None, 0)
assert_eq(0 or None, None)

assert_eq(0 and 1, 0)
assert_eq(0 or 1, 1)

assert_eq(1 and 0, 0)
assert_eq(1 or 0, 1)

assert_eq(1 and 2, 2)
assert_eq(1 or 2, 1)


# Short circuit
a = []
def f(x):
  a.append(x)
  return x

assert_eq(f(0) and f(None), 0)
assert_eq(a, [0])
a.clear()
assert_eq(f(1) or f(2), 1)
assert_eq(a, [1])

