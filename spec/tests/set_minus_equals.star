
# With another set.
def test1():
  a = set([0, 1, 3])
  b = set([3, 0, 2])
  a -= b
  assert_eq(a, set([1]))

test1()


# With self.
def test2():
  a = set([0, 1, 3])
  a -= a
  assert_eq(a, set())

test2()


# With a non-set.
assert_fail('''
def test3():
  a = set([0, 1, 3])
  a -= ()

test3()
''')


# While iterating.
assert_fail('''
def test4():
  a = set([0, 1, 3])
  for x in a:
    a -= a

test4()
''')

