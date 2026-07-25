# No arguments.
assert_eq(zip(), [])


# One argument.
assert_eq(zip([]), [])
assert_eq(zip((1, 2, 3)), [(1,), (2,), (3,)])
assert_fail('''zip(1)''')
assert_fail('''zip(None)''')


# Two arguments.
assert_eq(zip([1, 2, 3], [4, 5, 6]), [(1, 4), (2, 5), (3, 6)])
assert_eq(zip([1, 2, 3, 4], [4, 5, 6]), [(1, 4), (2, 5), (3, 6)])
assert_eq(zip([1, 2, 3, 10], [4, 5, 6]), [(1, 4), (2, 5), (3, 6)])
assert_eq(zip([1, 2, 3], [4, 5, 6, 10]), [(1, 4), (2, 5), (3, 6)])


# Three arguments.
assert_eq(zip(range(3), range(100, 200), range(1000, 1002)), [(0, 100, 1000), (1, 101, 1001)])


# Named arguments.
assert_fail('''zip(iterables = [])''')



# Zip creates a new list, so it is ok to remove from `a`
def foo():
  a = ['one', 'two', 'three']
  for (y,) in zip(a):
    a.remove(y)
  assert_eq(a, [])
foo()

