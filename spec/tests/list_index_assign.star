a = [0, 1, 2]
a[-2] = 3
assert_eq(a, [0, 3, 2])
a[2] = 4
assert_eq(a, [0, 3, 4])

assert_fail('''
a = [1]
a[2] = 2
''')

assert_fail('''
a = [1]
a[-2] = 2
''')

assert_fail('''
def foo():
  a = [1]
  for x in a:
    a[0] = 0

foo()
''')

assert_fail('''
## Begin module: "//:test1.bzl"
a = [0, 1, 2]
## End module
## Main
load("//:test1.bzl", "a")
a[0] = 3
''')

