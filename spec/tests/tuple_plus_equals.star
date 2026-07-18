def test():
  a = (0, 1)
  a += (3, 4, 5)
  assert_eq(a, (0, 1, 3, 4, 5))

test()


assert_fail('''
def test():
  a = ()
  a += []

test()
''')

