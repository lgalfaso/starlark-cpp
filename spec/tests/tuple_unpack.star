[a, b] = (1, 2)
assert_eq(a, 1)
assert_eq(b, 2)
x, y = 3, 4
assert_eq(x, 3)
assert_eq(y, 4)


assert_fail('''
[a, b] = (1,)
''')
assert_fail('''
[a, b] = (1, 2, 3)
''')

