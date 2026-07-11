
assert_eq([x for x in set([0, 1, 2, 3, 5, 4, 7, 6])], [0, 1, 2, 3, 5, 4, 7, 6])

assert_fail('''
def test():
  a = set([0])
  for x in a:
    a.update([x])

test()
''')

