# Mutate while in a loop
def foo():
  a = set([1])
  for x in a:
    a.add(x + 1)
foo()
