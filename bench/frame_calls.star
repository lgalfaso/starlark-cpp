"""Shallow function calls for frame-setup benchmarking."""

def f0():
    return 1

def f1(x):
    return x

def f2(x, y):
    return x + y

def f3(x, y, z):
    return x + y + z

def bench(n):
    s = 0
    for i in range(n):
        s = s + f0() + f1(i) + f2(i, i + 1) + f3(i, i + 1, i + 2)
    return s

print(bench(50000))
