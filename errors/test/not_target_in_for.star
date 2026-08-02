# Loop variables in a `for` statement must be valid targets.
def foo():
  a = []
  for a() in []: pass

