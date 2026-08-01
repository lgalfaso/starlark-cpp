# The name of the named arguments must be valid identifiers.
def abc(): pass
def foo(**kwargs): pass
foo(abc() = True)
