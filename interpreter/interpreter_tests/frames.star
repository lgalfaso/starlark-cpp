def foo():
  count = [0]
  def inc():
    count[0] += 1
    return count[0]
  def dec():
    count[0] -= 1
    return count[0]
  return [inc, dec]

x = foo()
assert_eq(x[0](), 1)
assert_eq(x[0](), 2)
assert_eq(x[1](), 1)
assert_eq(x[1](), 0)
