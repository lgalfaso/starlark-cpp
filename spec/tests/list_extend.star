a = [1, 2, 3]
assert_eq(a.extend([5, 6]), None)
assert_eq(a, [1, 2, 3, 5, 6])
assert_eq(a.extend(a), None)
assert_eq(a, [1, 2, 3, 5, 6, 1, 2, 3, 5, 6])
assert_eq(a.extend((88, 99)), None)
assert_eq(a, [1, 2, 3, 5, 6, 1, 2, 3, 5, 6, 88, 99])

assert_fail('''
a.extend()
''')

assert_fail('''
a.extend([], [])
''')

assert_fail('''
a.extend(True)
''')

assert_fail('''
def foo():
  a = [1, 2]
  for x in a:
    a.extend([x])

foo()
''')

