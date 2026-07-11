# No arguments.
assert_fail('''set().add()''')


# One argument.
a = set([1])
a.add(1.0)  # Adding an element that is equivalent is a no-op.
assert_eq(str(a), 'set([1])')

assert_eq(a.add(0), None)
assert_eq(str(a), 'set([1, 0])')


# While iterating.
assert_fail('''
def test():
  a = set([1])
  for x in a:
    a.add(None)

test()
''')


# Two arguments.
assert_fail('''set().add(0, 1)''')


# Named arguments.
assert_fail('''set().add(elem = 0)''')

