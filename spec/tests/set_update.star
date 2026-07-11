# No arguments.
a = set([0, 1, 2])
assert_eq(a.update(), None)
assert_eq(a, set([0, 1, 2]))


# One argument.
assert_eq(a.update(set([2, 4])), None)
assert_eq(a, set([0, 1, 2, 4]))


# Two arguments.
assert_eq(a.update(set([5, 4]), set([1, 8])), None)
assert_eq(a, set([0, 1, 2, 4, 5, 8]))


# Three arguments including self.
assert_eq(a.update(set([9, 5]), a, set([1, 10])), None)
assert_eq(a, set([0, 1, 2, 4, 5, 8, 9, 10]))


# Other iterable.
assert_eq(a.update([3, 13, 5]), None)
assert_eq(a, set([0, 1, 2, 4, 5, 8, 9, 10, 3, 13]))


# Non-hashable.
assert_fail('''set().update([1, set(), 2])''')
# Non-iterable.
assert_fail('''set().update(1)''')
assert_fail('''set().update(None)''')


# While iterating.
assert_fail('''
def test():
  a = set([0])
  for x in a:
    a.update()
test()
''')


# Named arguments.
assert_fail('''set().update(others = [])''')

