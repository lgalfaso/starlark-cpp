# No arguments.
a = set([0, 1])

assert_eq(a.difference_update(), None)
assert_eq(a, set([0, 1]))


# One argument
assert_eq(a.difference_update(set([0, -1])), None)
assert_eq(a, set([1]))
assert_eq(a.update([0, 1, -1]), None)
assert_eq(a.difference_update([-1, 1]), None)
assert_eq(a, set([0]))
assert_fail('''set().difference_update(None)''')
assert_fail('''set().difference_update([set()])''')
assert_fail('''set().difference_update(others = [])''')


# Two arguments.
assert_eq(a.update([0, 1, -1]), None)
assert_eq(a.difference_update(set([-1]), set([1])), None)
assert_eq(a, set([0]))



# Three arguments including self.
assert_eq(a.difference_update(set([-1]), a, set([1])), None)
assert_eq(a, set())



# While iterating.
assert_fail('''
def test():
  a = set([1])
  for x in a:
    a.difference_update()

test()
''')

