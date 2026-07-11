# No arguments.
assert_fail('''set([0]).symmetric_difference()''')


# One argument.
a = set([0, 1, 2])
assert_eq(a.symmetric_difference(set([2, 3, 4])), set([0, 1, 3, 4]))
assert_eq(a, set([0, 1, 2]))
assert_eq(a.symmetric_difference(a), set())
assert_eq(a, set([0, 1, 2]))
assert_eq(a.symmetric_difference([2, 3, 4]), set([0, 1, 3, 4]))
assert_eq(a, set([0, 1, 2]))
assert_fail('''set().symmetric_difference([0, None, set()])''')
assert_fail('''set().symmetric_difference(0)''')
assert_fail('''set().symmetric_difference(None)''')

# While iterating
def test():
  for x in a:
    print(a.symmetric_difference(set()))
test()


# Two oarguments.
assert_fail('''set().symmetric_difference(set(), set())''')


# Named arguments.
assert_fail('''set().symmetric_difference(other = set())''')

