def foo1():
  return 1
def foo2():
  pass
def foo3(a):
  return a + 1
def foo4(a, b = 1):
  return a(b)
def foo5(a, *b):
  return a(*b)
def foo6(a, **b):
  return a(**b)
def foo7(a, *, b = 2, c):
  print(a, b, c)
def foo8(*a, b):
  print(b, *a)
