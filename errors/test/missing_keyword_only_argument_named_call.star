# Missing keyword only argument via named call
def foo(a, *, bar):
  pass

foo(a=1)
