
a = []
def fn(x):
  a.append(x)
  return 1


assert_eq(fn(True), 1)
assert_eq(a, [True])



# No parameters.
def fn_1():
  pass

fn_1()



# One parameter.
b = []
def fn_2(x):
  b.append(x)

fn_2(1)
fn_2(x = 2)
assert_eq(b, [1, 2])



# Two parameters.
def idiv(x, y):
  return x // y

assert_eq(idiv(6, 3), 2)
assert_eq(idiv(x=6, y=3), 2)
assert_eq(idiv(y=3, x=6), 2)
assert_eq(idiv(6, y=3), 2)


def f(x, y=3):
  return x, y

assert_eq(f(1, 2), (1, 2))
assert_eq(f(1), (1, 3))


def g(x, y = []):
  y.append(x)
  return y

assert_eq(g(1), [1])
assert_eq(g(2), [1, 2])
assert_eq(g(3), [1, 2, 3])


assert_fail('''
## Begin module: "//:test1.bzl"
def foo(x, y = []):
  y.append(x)

## End module
## Main
load("//:test1.bzl", "foo")
foo(1)
''')


# Optional parameters must follow non-optional parameters.
assert_fail('''
def h(x = [], y):
  pass
''', allow_static_error = True)

def h1(*args, x = [], y):
  pass

def h2(x = [], *args, y):
  pass

def i(x, y, *args):
  return x, y, args

assert_eq(i(1, 2), (1, 2, ()))
assert_eq(i(1, 2, 3, 4), (1, 2, (3, 4)))

def j(x, y, **kwargs):
  return x, y, kwargs

assert_eq(j(1, 2), (1, 2, {}))
assert_eq(j(x=2, y=1), (2, 1, {}))
assert_eq(j(x=2, y=1, z=3), (2, 1, {"z": 3}))


# It is a static error to have the same parameter name in a function definition.
assert_fail('''
foo(x, x):
  pass
''', allow_static_error = True)
assert_fail('''
foo(x, *x):
  pass
''', allow_static_error = True)
assert_fail('''
foo(x, **x):
  pass
''', allow_static_error = True)
assert_fail('''
foo(x, *, x):
  pass
''', allow_static_error = True)
assert_fail('''
foo(x, *, y, x):
  pass
''', allow_static_error = True)
assert_fail('''
foo(x, *, y, **x):
  pass
''', allow_static_error = True)


def k(a, b, c=5):
  return a * b + c

assert_eq(k(*[2, 3]), 11)
assert_eq(k(*[2, 3, 7]), 13)

assert_fail('''
def k(a, b, c=5):
  return a * b + c

k(*[2])
''')

assert_eq(k(**dict(b=3, a=2)), 11)
assert_eq(k(**dict(c=7, a=2, b=3)), 13)

assert_fail('''
def k(a, b, c=5):
  return a * b + c

k(**dict(a=2))
''')

assert_fail('''
def k(a, b, c=5):
  return a * b + c

k(**dict(d=4))
''')


assert_fail('''
def f(x):
  pass

f(x=1, **dict(x=2))
''')


# Arguments are evaluated in the order they appear in the call.
l0 = []
def l1(x, y):
  pass
def l2(x):
  l0.append(x)
  return x

l1(x = l2(0), y = l2(1))
assert_eq(l0, [0, 1])
l0.clear()
l1(y = l2(0), x = l2(1))
assert_eq(l0, [0, 1])


# Position of *args in call
def m(*args, x):
  pass

m(10, x = 20, *[30, 40])

assert_fail('''
def m(*args, x, *more):
  pass
''', allow_static_error = True)

assert_fail('''
def m(*args, x):
  pass

m(10, *[30, 40], x = 20)
''', allow_static_error = True)


def n(x):
  if x == 0:
    return
  if x < 0:
    return -x
  print(x)

assert_eq(n(1), None)
assert_eq(n(0), None)
assert_eq(n(-1), 1)


assert_fail('''
def fib(x):
  if x <= 1:
    return x
  return fib(x - 1) + fib(x - 2)

print(fib(3))
''')

r = []
append = r.append
assert_eq(append(1), None)
assert_eq(r, [1])



def s(a, d = 1, *args, b, c = 5):
  return (a, b, c, d, args)

assert_eq(s(1, 4, b=3), (1, 3, 5, 4, ()))
assert_eq(s(1, b=3, *[4, 5]), (1, 3, 5, 4, (5,)))


