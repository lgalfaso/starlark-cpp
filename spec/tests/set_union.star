# No arguments.
a = set([0, 1, 2])
assert_eq(a.union(), set([0, 1, 2]))
assert_eq(a, set([0, 1, 2]))


# One argument.
assert_eq(a.union(set([2, 4])), set([0, 1, 2, 4]))
assert_eq(a, set([0, 1, 2]))
assert_eq(a.union([2, 4]), set([0, 1, 2, 4]))
assert_eq(a, set([0, 1, 2]))
assert_fail('''set().union([0, set(), 2])''')
assert_fail('''set().union(0)''')
assert_fail('''set().union(None)''')

# While iterating.
def test():
  for x in a:
    print(a.union([x + 1]))
test()


# Two arguments.
assert_eq(a.union(set([2, 4]), set([2, 6])), set([0, 1, 2, 4, 6]))
assert_eq(a, set([0, 1, 2]))


# Three arguments including self.
assert_eq(a.union(set([2, 4]), a, set([2, 6])), set([0, 1, 2, 4, 6]))
assert_eq(a, set([0, 1, 2]))


# Named argument.
assert_fail('''set().union(others = [])''')

