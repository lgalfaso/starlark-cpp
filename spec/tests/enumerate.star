# No arguments.
assert_fail('''enumerate()''')


# One argument.
assert_eq(enumerate([]), [])
assert_eq(enumerate(['zero']), [(0, 'zero')])
assert_eq(enumerate(['zero', 'one']), [(0, 'zero'), (1, 'one')])
assert_fail('''enumerate(None)''')
assert_fail('''enumerate(1)''')
assert_fail('''enumerate(True)''')


# Two arguments.
assert_eq(enumerate([], 100), [])
assert_eq(enumerate(['zero'], 100), [(100, 'zero')])
assert_eq(enumerate(['zero', 'one'], 100), [(100, 'zero'), (101, 'one')])
assert_fail('''enumerate([], None)''')
assert_fail('''enumerate([], '')''')


# Three arguments.
assert_fail('''enumerate([], 100, None)''')


# Named arguments.
assert_eq(enumerate(iterable = []), [])
assert_eq(enumerate(iterable = ['zero']), [(0, 'zero')])
assert_eq(enumerate(iterable = ['zero', 'one']), [(0, 'zero'), (1, 'one')])
assert_fail('''enumerate(iterable = [], start = None)''')
assert_fail('''enumerate(iterable = [], start = '')''')


# Mixed positional and named arguments.
assert_eq(enumerate([], start = 100), [])
assert_eq(enumerate(['zero'], start = 100), [(100, 'zero')])
assert_eq(enumerate(['zero', 'one'], start = 100), [(100, 'zero'), (101, 'one')])
assert_fail('''enumerate([], start = None)''')
assert_fail('''enumerate([], start = '')''')
assert_fail('''enumerate([], iterable = [])''')
assert_fail('''enumerate([], 100, start = 100)''')
assert_fail('''enumerate([], unknown = None)''')


# Enumerate creates a new list, so it is ok to remove from `a`
def foo():
  a = ['one', 'two', 'three']
  for x, y in enumerate(a):
    a.remove(y)
  assert_eq(a, [])
foo()
