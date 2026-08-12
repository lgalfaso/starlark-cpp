# Mutating while in a loop
def foo():
  a = {1: 1}
  for x in a: 
    a.pop(x)
foo()
