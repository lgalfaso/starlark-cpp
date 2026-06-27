assert_fail('''
[1, 2, 3].remove()
''')

l = [0, 1, 0, 0, 1, 1]
assert_eq(l.remove(1), None)
assert_eq(l, [0, 0, 0, 1, 1])

assert_fail('''
[1, 2, 3].remove(0)
''')

assert_fail('''
def foo():
  a = [1, 2, 3]
  for x in a:
    a.remove(x)

foo()
''')

assert_fail('''
[1, 2, 3].remove(1, 2)
''')

assert_fail('''
[1, 2, 3].remove(value = 1)
''')
