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
""", error_message = """foo expected 0 arguments, got 1
    4 | foo(1)
      | ~~~^^^
""")

assert_fail("""
def foo(a):
  pass

foo(1, a = 2)
""", error_message = """foo() got multiple values for argument 'a'
    4 | foo(1, a = 2)
      | ~~~^^^^^^^^^^
""")

assert_fail("""
def foo(a):
  pass

foo(b = 2)
""", error_message = """foo() got an unexpected keyword argument 'b'
    4 | foo(b = 2)
      | ~~~^^^^^^^
""")

assert_fail("""
def foo(a):
  pass

foo()
""", error_message = """foo() missing required positional argument 'a'
    4 | foo()
      | ~~~^^
""")

assert_fail("""
def foo(*, a):
  pass

foo()
""", error_message = """foo() missing required keyword-only argument 'a'
    4 | foo()
      | ~~~^^
""")

assert_fail("""
def foo(a, *, bar):
  pass

foo(a=1)
""", error_message = """foo() missing required keyword-only argument 'bar'
    4 | foo(a=1)
      | ~~~^^^^^
""")

x = []
def y(p1, p2 = 2, *, p3 = 3, p4):
  x.append(p1)
  x.append(p2)
  x.append(p3)
  x.append(p4)

y(1, p4 = 4)
assert_eq(x, [1, 2, 3, 4])

# Keyword-only defaults applied on the positional call fast path (CallPos0-3).
def kwonly0(*, a = 10, b = 20):
  return a + b

assert_eq(kwonly0(), 30)

def kwonly1(x, *, a = 10):
  return x + a

assert_eq(kwonly1(1), 11)

def kwonly2(x, y, *, a = 10):
  return x + y + a

assert_eq(kwonly2(1, 2), 13)

def kwonly3(x, y, z, *, a = 10):
  return x + y + z + a

assert_eq(kwonly3(1, 2, 3), 16)

# Positional defaults precede keyword-only defaults in the default-argument list.
def kwonly_offset(x, y = 5, *, a = 10, b = 20):
  return (x, y, a, b)

assert_eq(kwonly_offset(1, 2), (1, 2, 10, 20))

# Keyword-only defaults on the general positional path (4+ positional params).
def kwonly4(a, b, c, d, *, x = 10, y = 20):
  return (a, b, c, d, x, y)

assert_eq(kwonly4(1, 2, 3, 4), (1, 2, 3, 4, 10, 20))

# Fewer positional args than arity, plus keyword-only defaults.
def kwonly_partial(a, b = 5, *, x = 10):
  return (a, b, x)

assert_eq(kwonly_partial(1), (1, 5, 10))

# Keyword-only defaults with *args (no positional fast path).
def kwonly_star(a, *args, x = 10):
  return (a, args, x)

assert_eq(kwonly_star(1, 2, 3), (1, (2, 3), 10))

# Keyword-only defaults with **kwargs (no positional fast path).
def kwonly_starstar(*, x = 10, **kwargs):
  return (x, kwargs)

assert_eq(kwonly_starstar(), (10, {}))

# Multiple keyword-only defaults on call_pos_general; default_argument_pos must
# advance for each keyword-only parameter that has a default.
def kwonly_general_defaults(a, b, c, d = 40, *, x = 10, y = 20):
  return (a, b, c, d, x, y)

assert_eq(kwonly_general_defaults(1, 2, 3), (1, 2, 3, 40, 10, 20))

def kwonly_two_defaults(a, b, c, d, *, w = 1, x = 2, y = 3, z = 4):
  return (a, b, c, d, w, x, y, z)

assert_eq(kwonly_two_defaults(1, 2, 3, 4), (1, 2, 3, 4, 1, 2, 3, 4))
