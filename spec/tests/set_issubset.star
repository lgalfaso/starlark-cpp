a = set([1, 2])
b = set([1, 2, 3])
c = set([0, 2, 3])
d = set([2])
e = set([1, 2])
f = set([3, 4])

# No arguments.
assert_fail('''set().issubset()''')


# One argument.
assert_true(a.issubset(a))
assert_true(a.issubset(b))
assert_false(a.issubset(c))
assert_false(a.issubset(d))
assert_true(a.issubset(e))
assert_false(a.issubset(f))

assert_fail("""set([1, 2]).issubset([1, 2, set()])""")
assert_fail("""set([1, 2]).issubset([set()])""")
assert_fail("""set([1, 2]).issubset(None)""")

assert_true(a.issubset((1, 2, 3)))


# Two arguments.
assert_fail('''set().issubset(set(), set())''')


# While iterating.
def test():
  for x in a:
    a.issubset(a)

test()


# Named arguments.
assert_fail('''set().issubset(other = set())''')
