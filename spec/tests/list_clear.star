a = [0, 1, 2]
assert_eq(a.clear(), None)
assert_eq(a, [])

assert_fail('''
[].clear(0)
''')

assert_fail('''
[].clear(pos = 0)
''')

assert_fail('''
def foo():
  a = [0, 1, 2]
  for x in a:
    a.clear()

foo()
''')

