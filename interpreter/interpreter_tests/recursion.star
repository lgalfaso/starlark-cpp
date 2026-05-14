assert_fail("""
def fibonacci(n):
  if n < 2:
    return n
  return fibonacci(n - 1) + fibonacci(n - 2)

print(fibonacci(6))
""", error_message = "Error: function 'fibonacci' called recursively")

assert_succeed("""
def fibonacci(n):
  if n < 2:
    return n
  return fibonacci(n - 1) + fibonacci(n - 2)

print(fibonacci(6))
""", allow_recursion = True, print = "8\n")
