a = []

def foo():
  a.append(1)
  assert_eq(a, [1])

foo()

# Once an element is frozen, it cannot be modified, not even from within the same module.
assert_fail('''
## Begin module: "//:test1.bzl"
a = [0, 1, 2]

def foo():
  a.append(1)

## End module
## Main
load("//:test1.bzl", "foo")
foo()
''')

