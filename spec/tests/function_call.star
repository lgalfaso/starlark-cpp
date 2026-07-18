
a = []
def fn(x):
  a.append(x)
  return 1


assert_eq(fn(True), 1)
assert_eq(a, [True])


# TODO(lmirelmann): Add the tests for all the parameter types and variations.


