# Tuples are immutable.
assert_fail('''
a = (1, 2)
a[0] = None
''')
