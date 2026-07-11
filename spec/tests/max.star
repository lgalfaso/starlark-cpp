
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




