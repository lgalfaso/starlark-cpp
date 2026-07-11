# No arguments.
a = set([0, 1])
b = set([0, 1, 2, 3])
assert_eq(a.difference(), set([0, 1]))


# One argument.
assert_eq(a.difference(set([0, -1])), set([1]))
assert_eq(a.difference([0, -1]), set([1]))
assert_fail('''set([1]).difference([set()])''')
assert_fail('''set([1]).difference(None)''')
assert_fail('''set([1]).difference(1)''')

# Two arguments.
assert_eq(b.difference(set([0, 2]), set([0, 3])), set([1]))


# Self as a parameter.
assert_eq(b.difference(set([5]), b), set())


# Named parameter
assert_fail('''set().difference(others = [])''')


# While iterating
def test():
  for x in a:
    print(a.difference(b))

test()

