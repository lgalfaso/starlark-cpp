# No arguments.
assert_fail('''max()''')


# One argument.
assert_fail('''max([])''')
assert_eq(max([1]), 1)
assert_eq(max([1, 2]), 2)
assert_eq(max([1, 3, 2]), 3)
assert_fail('''max([1, '2'])''')
assert_fail('''max(1)''')
assert_fail('''max(True)''')
assert_fail('''max(None)''')


# Two or more positional arguments.
assert_fail('''max([1], None)''')
assert_eq(max(1, 2), 2)
assert_eq(max(1, 3, 2), 3)
assert_fail('''max(1, True)''')


# Named arguments.
assert_eq(max(['one', 'two', 'three'], key = len), 'three')
assert_eq(max(['one', 'two', 'three'], key = None), 'two')
assert_fail('''max([1, 'two', 'three'], key = len)''')
assert_fail('''max(['one', 2, 'three'], key = len)''')
assert_fail('''max([[], []], key = set)''')

assert_eq(max('one', 'two', 'three', key = len), 'three')
assert_eq(max('one', 'two', 'three', key = None), 'two')
assert_fail('''max(1, 'two', 'three', key = len)''')
assert_fail('''max('one', 2, 'three', key = len)''')
assert_fail('''max([], [], key = set)''')
assert_fail('''max(iterable = [1])''')


# https://github.com/bazelbuild/bazel/issues/30201
assert_fail('''
def test1():
  a = list(range(100, 200))
  b = [0]
  def x(r):
    a[b[0]] = b[0]
    b[0] += 1
    return b[0]
  max(a, key = x)
test1()
''')

assert_fail('''
def test2():
  a = list(range(100, 200))
  b = [0]
  def x(q):
    a[(b[0] + 1) % len(a)] = q + b[0]
    b[0] += 1
    return b[0]
  max(a, key = x)
test2()
''')




