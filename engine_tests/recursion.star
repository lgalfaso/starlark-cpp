assert_fail("""
def fibonacci(n):
  if n < 2:
    return n
  return fibonacci(n - 1) + fibonacci(n - 2)

print(fibonacci(6))
""", error_message = """function 'fibonacci' called recursively
    4 |   return fibonacci(n - 1) + fibonacci(n - 2)
      |          ~~~~~~~~~^^^^^^^
""")

assert_succeed("""
def fibonacci(n):
  if n < 2:
    return n
  return fibonacci(n - 1) + fibonacci(n - 2)

print(fibonacci(6))
""", allow_recursion = True, print = "8\n")

assert_fail("""
def foo(a, b, c):
  return foo(a, b, c)

foo(1, 2, 3)
""", error_message = """function 'foo' called recursively
    2 |   return foo(a, b, c)
      |          ~~~^^^^^^^^^
""")

assert_fail("""
def foo(a, b, c, d):
  return foo(a, b, c, d)

foo(1, 2, 3, 4)
""", error_message = """function 'foo' called recursively
    2 |   return foo(a, b, c, d)
      |          ~~~^^^^^^^^^^^^
""")

assert_fail("""
def foo(a):
  return foo(a=a)

foo(1)
""", error_message = """function 'foo' called recursively
    2 |   return foo(a=a)
      |          ~~~^^^^^
""")
