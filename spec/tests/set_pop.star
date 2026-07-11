a = set([0, 1, 2, 4, 3])
# No arguments.
assert_eq(a.pop(), 0)
assert_eq(a.pop(), 1)
assert_eq(a.pop(), 2)
assert_eq(a.pop(), 4)
assert_eq(a.pop(), 3)
assert_eq(a, set())

assert_fail('''set().pop()''')

# While iterating.
assert_fail('''
def test():
  a = set([0])
  for x in a:
    a.pop()

test()
''')


# One argument.
assert_fail('''set([0, 1]).pop(None)''')
assert_fail('''set([0, 1]).pop(0)''')


# Named arguments.
assert_fail('''set([0, 1]).pop(index = 0)''')

