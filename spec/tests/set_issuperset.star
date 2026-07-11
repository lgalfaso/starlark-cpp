a = set([1, 2])
b = set([1, 2, 3])
c = set([0, 2, 3])
d = set([2])
e = set([1, 2])
f = set([3, 4])

# No arguments.
assert_fail('''set().issuperset()''')


# One argument.
assert_true(a.issuperset(a))
assert_false(a.issuperset(b))
assert_false(a.issuperset(c))
assert_true(a.issuperset(d))
assert_true(a.issuperset(e))
assert_false(a.issuperset(f))

assert_fail("""set([1, 2]).issuperset([1, 2, set()])""")
assert_fail("""set([1, 2]).issuperset([set()])""")
assert_fail("""set([1, 2]).issuperset(None)""")
assert_fail("""set([1, 2]).issuperset(1)""")

assert_true(a.issuperset((1, 2)))
assert_false(a.issuperset((2, 4, 3)))


# Two arguments.
assert_fail('''set().issuperset(set(), set())''')


# While iterating.
def test():
  for x in a:
    a.issuperset(a)

test()


# Named arguments.
assert_fail('''set().issuperset(other = set())''')

