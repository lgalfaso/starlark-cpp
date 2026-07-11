a = set([1, 2])
b = set([1, 2, 3])
c = set([0, 2, 3])
d = set([2])
e = set([1, 2])
f = set([3, 4])

# No arguments.
assert_fail('''set().isdisjoint()''')


# One argument.
assert_false(a.isdisjoint(a))
assert_false(a.isdisjoint(b))
assert_false(a.isdisjoint(c))
assert_false(a.isdisjoint(d))
assert_false(a.isdisjoint(e))
assert_true(a.isdisjoint(f))

assert_fail("""set([1, 2]).isdisjoint([1, 2, set()])""")
assert_fail("""set([1, 2]).isdisjoint([set()])""")
assert_fail("""set([1, 2]).isdisjoint(None)""")
assert_fail("""set([1, 2]).isdisjoint(1)""")

assert_false(a.isdisjoint((1, 2, 3)))
assert_true(a.isdisjoint((3, 4, 3)))


# Two arguments.
assert_fail('''set().isdisjoint(set(), set())''')


# While iterating.
def test():
  for x in a:
    a.isdisjoint(a)

test()


# Named arguments.
assert_fail('''set().isdisjoint(other = set())''')

