def foo():
  def bar():
    pass
  return bar

def man():
  pass

a = foo()

assert_eq(foo == foo, True)
assert_eq(man == man, True)
assert_eq(man == foo, False)
assert_eq(man == len, False)
assert_eq(foo() == foo(), False)
assert_eq(a == a, True)

assert_true([man] <= [man])
assert_true([a] <= [a])
assert_fail("""
def man():
  pass

man <= man
""")
assert_fail("""
def man():
  pass

def foo():
  pass

[man] <= [foo]
""")
assert_fail("""
def foo():
  def bar():
    pass
  return bar

[foo()] <= [foo()]
""")
