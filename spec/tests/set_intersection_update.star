a = set([0, 1, 2])

# No arguments.
assert_eq(a.intersection_update(), None)
assert_eq(a, set([0, 1, 2]))


# One argument.
assert_eq(a.intersection_update(set([-1, 0, 1])), None)
assert_eq(a, set([0, 1]))
a.update([0, 1, 2])
# Other iterable.
assert_eq(a.intersection_update([-1, 0, 1]), None)
assert_eq(a, set([0, 1]))
# Other iterable, with non-hashable.
assert_fail('''set().intersection_update([set()])''')
# Non-iterable.
assert_fail('''set().intersection_update(None)''')
assert_fail('''set().intersection_update(1)''')

# Two arguments.
a.update([0, 1, 2])
assert_eq(a.intersection_update(set([1, 2, 3]), set([-1, 0, 1, 3])), None)
assert_eq(a, set([1]))


# Many arguments including self.
a.update([0, 1, 2])
assert_eq(a.intersection_update(set([1, 2, 3]), a, set([-1, 0, 1, 3])), None)
assert_eq(a, set([1]))


# Named arguments.
assert_fail('''set().intersection_update(others = [])''')


# While iterating.
assert_fail('''
def test():
  a = set([1])
  for x in a:
    print(a.intersection_update([1]))

test()
''')

