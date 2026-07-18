def test1():
  a = ()
  a *= 2
  assert_eq(a, ())
  a = ()
  a *= 1 << 64
  assert_eq(a, ())
  a = (0, 1)
  a *= 2
  assert_eq(a, (0, 1, 0, 1))
  a = (0, 1)
  a *= 3
  assert_eq(a, (0, 1, 0, 1, 0, 1))
  a = (0, 1)
  a *= 0
  assert_eq(a, ())
  a = (0, 1)
  a *= -1
  assert_eq(a, ())
  a = (0, 1)
  a *= -2
  assert_eq(a, ())

test1()


def test2():
  a = 2
  a *= ()
  assert_eq(a, ())
  a = 1 << 64
  a *= ()
  assert_eq(a, ())
  a = 2
  a *= (0, 1)
  assert_eq(a, (0, 1, 0, 1))
  a = 3
  a *= (0, 1)
  assert_eq(a, (0, 1, 0, 1, 0, 1))
  a = 0
  a *= (0, 1)
  assert_eq(a, ())
  a = -1
  a *= (0, 1)
  assert_eq(a, ())
  a = -2
  a *= (0, 1)
  assert_eq(a, ())

test2()


assert_fail('''
def test():
  a = ()
  a *= None

test()
''')

