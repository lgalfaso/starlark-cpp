# No arguments.
assert_fail('''set().symmetric_difference_update()''')


# One argument.
a = set([0, 1, 2])
assert_eq(a.symmetric_difference_update(set([2, 3])), None)
assert_eq(a, set([0, 1, 3]))
assert_eq(a.symmetric_difference_update(a), None)
assert_eq(a, set())
a.update([0, 1, 2])
assert_eq(a.symmetric_difference_update([2, 3]), None)
assert_eq(a, set([0, 1, 3]))

# Non-hashable.
assert_fail('''set([0, 1]).symmetric_difference_update([1, set()])''')
# Non-iterable.
assert_fail('''set([0, 1]).symmetric_difference_update(1)''')
assert_fail('''set([0, 1]).symmetric_difference_update(None)''')


# While iterating.
assert_fail('''
def test():
  a = set([0])
  for x in a:
    a.symmetric_difference_update([])
test()
''')


# Two arguments.
assert_fail('''set().symmetric_difference_update(set(), set())''')


# Named argument.
assert_fail('''set().symmetric_difference_update(other = set())''')

