
def fn():
  pass

a = lambda: True

# Check that all functions are hashable.
x = {fn: 'function', a: 'lambda', len: 'builtin function'}


