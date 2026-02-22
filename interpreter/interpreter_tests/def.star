def foo():
  return "a"

def bar():
  x = [[]]
  def inner():
    x[0].append('x')
    return x[0]
  return inner

c = bar();

assert_eq(foo(), "a")
assert_eq(c(), ['x'])
assert_eq(c(), ['x', 'x'])

