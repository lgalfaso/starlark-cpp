
assert_eq([x * x for x in [0, 1, 2, 3, 4]], [0, 1, 4, 9, 16])

assert_fail('''
def test():
  a = [1, 2]
  for x in a:
    a.pop()

test()
''')

