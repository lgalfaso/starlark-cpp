assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index()
''')

p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
assert_eq(p.index(2), 0)
assert_eq(p.index(17), 6)
assert_eq(p.index(19), 8)

assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(4)
''')


assert_eq(p.index(17, 0), 6)
assert_eq(p.index(17, 7), 7)
assert_eq(p.index(17, None), 6)

assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(5, True)
''')
assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(11, 1 << 64)
''')

assert_eq(p.index(5, 1, 3), 2)
assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(5, 1, 2)
''')
assert_eq(p.index(5, 1, 1 << 64), 2)

assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(5, 0, True)
''')
assert_eq(p.index(5, 0, -1), 2)
assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(19, 0, -1)
''')

assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(19, None, None, None)
''')
assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(value = 19)
''')
assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(19, start = 0)
''')
assert_fail('''
p = [2, 3, 5, 7, 11, 13, 17, 17, 19]
p.index(19, end = -1)
''')
