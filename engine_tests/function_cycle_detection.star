assert_fail('''
Y = lambda f: (lambda x: x(x))(lambda y: f(lambda *args: y(y)(*args)))
fibgen = lambda fib: lambda x: (x if x<2 else fib(x-1)+fib(x-2))
fib2 = Y(fibgen)
[fib2(x) for x in range(10)]
''')
