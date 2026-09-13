def foo1(a):
    return a

def foo2(a, b=1):
    return a + b

def foo3(a, *args):
    return a

def foo4(a, **kwargs):
    return a

def foo5(a, b, c=2, d=3, e=4):
    return a + b + c + d + e

lam0 = lambda: 1
lam1 = lambda a: a + 1
lam2 = lambda a, b=1: a + b

_ = (foo1(1), foo2(1), foo3(1, 2), foo4(1), foo5(1, 2), lam0(), lam1(1), lam2(1))
