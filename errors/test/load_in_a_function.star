# The `load` statement cannot be within a function.

def foo():
  load("//:test.start", "bar")

