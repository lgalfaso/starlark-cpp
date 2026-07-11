# No arguments.
assert_fail('''set([0, 1]).remove()''')


# One argument.
a = set([0, 1, 2])
assert_eq(a.remove(1), None)
assert_eq(a, set([0, 2]))
assert_eq(a.remove(2), None)
assert_eq(a, set([0]))

assert_fail('''set([0]).remove(1)''')

# While iterating.
assert_fail('''
def test():
  a = set([0, 1])
  for x in a:
    a.remove(x)

test()
''')

# Trying to remove an unhashable.
assert_fail('''set([0]).remove([])''')


# Two arguments.
assert_fail('''set([0, 1]).remove(0, 1)''')


# Named argument.
assert_fail('''set([0, 1, 2]).remove(elem = 0)''')

