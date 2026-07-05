assert_eq([x for x in {0: 1, 2: 3, 4: 5, 6: 7, 8: 9}], [0, 2, 4, 6, 8])

assert_fail('''
def test():
  a = {0: 1, 2: 3, 4: 5, 6: 7, 8: 9}
  for x in a:
    a[None] = None

test()
''')

