# Mutating while in a loop
def foo():
  a = {1: 1}
  for x in a: 
    a[x + 1] = x + 1
foo()
