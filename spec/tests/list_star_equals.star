
def test1():
  l1 = [0, 1, 2]
  l1 *= 2
  assert_eq(l1, [0, 1, 2, 0, 1, 2])

  l1 = [0, 1, 2]
  l1 *= 3
  assert_eq(l1, [0, 1, 2, 0, 1, 2, 0, 1, 2])

  l1 = [0, 1, 2]
  l1 *= 0
  assert_eq(l1, [])

  l1 = [0, 1, 2]
  l1 *= -1
  assert_eq(l1, [])

  l1 = [0, 1, 2]
  l1 *= -2
  assert_eq(l1, [])

  l2 = []
  l2 *= 3
  assert_eq(l2, [])
  l2 *= (1 << 64)
  assert_eq(l2, [])

test1()

def test2():
  l1 = 2
  l1 *= [0, 1, 2]
  assert_eq(l1, [0, 1, 2, 0, 1, 2])

  l1 = 3
  l1 *= [0, 1, 2]
  assert_eq(l1, [0, 1, 2, 0, 1, 2, 0, 1, 2])

  l1 = 0
  l1 *= [0, 1, 2]
  assert_eq(l1, [])

  l1 = -1
  l1 *= [0, 1, 2]
  assert_eq(l1, [])

  l1 = -2
  l1 *= [0, 1, 2]
  assert_eq(l1, [])

  l2 = 3
  l2 *= []
  assert_eq(l2, [])
  l2 = (1 << 64)
  l2 *= []
  assert_eq(l2, [])

test2()

assert_fail('''
def test():
  a = []
  a *= None

test()
''')

assert_fail('''
def test():
  a = []
  a *= True

test()
''')

assert_fail('''
def test():
  a = [1]
  for x in a:
    a *= x

test()
''')

