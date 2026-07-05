[a, b] = {1: None, 2: None}
assert_eq(a, 1)
assert_eq(b, 2)


assert_fail('''
[a, b] = {1: None}
''')

assert_fail('''
[a, b] = {1: None, 2: None, 3: None}
''')

