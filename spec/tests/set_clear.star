# No arguments.
a = set([0, 1, 3])

assert_eq(a.clear(), None)
assert_eq(a, set())


# While iterating.
assert_fail('''
def test():
  a = set([0])
  for x in a:
    a.clear()

test()
''')

assert_fail('''set().clear(None)''')
assert_fail('''set().clear(other = None)''')

