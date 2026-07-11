# No arguments.
assert_fail('''set().discard()''')


# One argument.
a = set([0, 1])

assert_eq(a.discard(1), None)
assert_eq(a, set([0]))

assert_eq(a.discard(2), None)
assert_eq(a, set([0]))


# While iterating.
assert_fail('''
def test():
  a = set([0])
  for x in a:
    a.discard(None)

test()
''')


# Non-hashable.
assert_fail('''set().discard(set())''')



# Named arguments.
assert_fail('''set().discard(elem = None)''')

