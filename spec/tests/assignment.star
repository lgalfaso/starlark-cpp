def f(): return [24, 12]

a, b = 2, 3
(x, y) = f()
[zero, one, two] = range(3)
[] = ()

assert_eq(a, 2)
assert_eq(b, 3)
assert_eq(x, 24)
assert_eq(y, 12)
assert_eq(zero, 0)
assert_eq(one, 1)
assert_eq(two, 2)
