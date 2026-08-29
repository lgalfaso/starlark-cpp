# Recursive call with named arguments
def foo(a):
  return foo(a=a)

foo(1)
