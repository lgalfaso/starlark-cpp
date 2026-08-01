# Duplicate binding between a `load` and a function.
load("//:test.star", "foo")
def foo(): pass
