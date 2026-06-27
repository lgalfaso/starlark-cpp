list = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16]
assert_eq(list.pop(), 16)
assert_eq(list, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15])
assert_eq(list.pop(), 15)
assert_eq(list, [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14])
assert_eq(list.pop(3), 3)
assert_eq(list, [0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14])
assert_eq(list.pop(-3), 12)
assert_eq(list, [0, 1, 2, 4, 5, 6, 7, 8, 9, 10, 11, 13, 14])

assert_fail('''
[0, 1, 2].pop(-4)
''')
assert_fail('''
[0, 1, 2].pop(3)
''')
assert_fail('''
def foo():
  a = [0, 1, 2]
  for x in a:
    a.pop()

foo()
''')
assert_fail('''
[].pop()
''')
assert_fail('''
[0].pop(1 << 64)
''')
assert_fail('''
[0].pop(False)
''')
assert_fail('''
[0].pop(False)
''')
assert_fail('''
[0].pop(None)
''')
assert_fail('''
[0].pop(0, 0)
''')
assert_fail('''
[0].pop(index = 0)
''')
