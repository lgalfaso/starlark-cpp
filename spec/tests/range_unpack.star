[a, b] = range(2)
assert_eq(a, 0)
assert_eq(b, 1)

assert_fail('''
[a, b] = range(1)
''')
assert_fail('''
[a, b] = range(3)
''')

