# Positional arguments must come before named arguments.
def foo(*args, **kwargs): pass
foo(a = 1, (True))
