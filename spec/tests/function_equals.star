def fn1():
  pass

def fn2():
  def inner():
    pass
  return inner;

def fn3():
  def inner():
    pass
  return inner;

assert_true(fn1 == fn1)
assert_false(fn1 != fn1)
assert_false(fn1 == fn2)
assert_true(fn1 != fn2)
assert_false(fn2() == fn2())
assert_true(fn2() != fn2())
assert_false(fn2() == fn3())
assert_true(fn2() != fn3())

i = fn2()
assert_true(i == i)
assert_false(i != i)

