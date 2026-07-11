a = set([0, 1, 2])
b = set([0, 2, 3])
c = set([0, 3, 4])
# No arguments.
assert_eq(a.intersection(), set([0, 1, 2]))


# One argument.
assert_eq(a.intersection(b), set([0, 2]))
assert_eq(a.intersection([0, 2, 3]), set([0, 2]))

# Non-hashable.
assert_fail('''set().intersection([set()])''')

# Non-iterable.
assert_fail('''set().intersection(None)''')
assert_fail('''set().intersection(1)''')

# While iterating
def test():
  for x in a:
    print(a.intersection(b))

# Two arguments.
assert_eq(a.intersection(b, c), set([0]))


# Three arguments including self.
assert_eq(a.intersection(b, a, c), set([0]))


# Named arguments.
assert_fail('''set().intersection(others = [])''')

