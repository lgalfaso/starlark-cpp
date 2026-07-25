a = []
def f(x):
  a.append(x)
  return x


assert_eq(f(1) if True else f(2), 1)
assert_eq(f(3) if False else f(4), 4)
assert_eq(a, [1, 4])

