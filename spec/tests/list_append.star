assert_fail('''
[].append()
''')

a = []
assert_eq(a.append(0), None)
assert_eq(a, [0])
assert_eq(a.append(2), None)
assert_eq(a, [0, 2])
assert_eq(a.append("hello"), None)
assert_eq(a, [0, 2, "hello"])

assert_fail('''
[].append(0, 0)
''')

assert_fail('''
[].append(value = 0)
''')

assert_fail('''
def foo():
  a = [0, 1, 2, 3, 4]
  for x in a:
    a.append("value")

foo()
''')

