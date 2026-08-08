def foo():
  return "a"

def bar():
  x = [[]]
  def inner():
    x[0].append('x')
    return x[0]
  return inner

c = bar();

def man(a, b):
  return a + b

def shell(a, b = 10):
  return a + b

def rock(a, b = []):
  b.append(a)
  return b

def make_tuple(*args):
  return args

def make_dict(**kwargs):
  return kwargs

assert_eq(foo(), "a")
assert_eq(c(), ['x'])
assert_eq(c(), ['x', 'x'])
assert_eq(man(1, 2), 3)
assert_eq(man(1, b = 3), 4)
assert_eq(man(a = 4, b = 7), 11)
assert_eq(shell(1, 2), 3)
assert_eq(shell(1, b = 3), 4)
assert_eq(shell(4), 14)
assert_eq(shell(a = 6), 16)
assert_eq(rock(1), [1])
assert_eq(rock(2), [1, 2])
assert_eq(rock(3, []), [3])
assert_eq(make_tuple(1, 2, 3, 4), (1, 2, 3, 4))
assert_eq(make_dict(a = 1, b = 2, c = 3, d = 4), {'a': 1, 'b': 2, 'c': 3, 'd': 4})


assert_fail("""
def foo():
  pass

foo(1)
""", error_message = """TypeError: foo expected 0 arguments, got 1
    4 | foo(1)
      | ~~~^^^
""")

assert_fail("""
def foo(a):
  pass

foo(1, a = 2)
""", error_message = """TypeError: foo() got multiple values for argument 'a'
    4 | foo(1, a = 2)
      | ~~~^^^^^^^^^^
""")

assert_fail("""
def foo(a):
  pass

foo(b = 2)
""", error_message = """TypeError: foo() got an unexpected keyword argument 'b'
    4 | foo(b = 2)
      | ~~~^^^^^^^
""")

assert_fail("""
def foo(a):
  pass

foo()
""", error_message = """TypeError: foo() missing required positional argument 'a'
    4 | foo()
      | ~~~^^
""")

assert_fail("""
def foo(*, a):
  pass

foo()
""", error_message = """TypeError: foo() missing required keyword-only argument 'a'
    4 | foo()
      | ~~~^^
""")

x = []
def y(p1, p2 = 2, *, p3 = 3, p4):
  x.append(p1)
  x.append(p2)
  x.append(p3)
  x.append(p4)

y(1, p4 = 4)
assert_eq(x, [1, 2, 3, 4])
