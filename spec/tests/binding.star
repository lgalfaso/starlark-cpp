# Load statements cannot load a global from another module.
assert_fail('''
## Begin module: "//:test1.bzl"
a = 1
## End module
## Main
load("//:test1.bzl", "None")
''')

# But can do so if this is the file block even if the same binding is global.
assert_succeed('''
## Begin module: "//:test1.bzl"
None = 1
## End module
## Main
load("//:test1.bzl", "None")
''')

# Module and file block cannot overlap.
assert_fail('''
## Begin module: "//:test1.bzl"
a = 1
## End module
## Main
load("//:test1.bzl", "a")

a = 2
''', allow_static_error = True)


# It is an error to reference a variable before it is bound.
assert_fail('''
print(a)
a = 2
''')

assert_fail('''
def f():
  print(x)
  x = "hello"
f()
''')


[1//0 for x in [] for y in z for z in ()]
assert_fail('''
[1//0 for x in [1] for y in z for z in ()]
''')

# Unknown binding.
assert_fail('''
def f():
  if False:
    g()
''', allow_static_error = True)

# Multiple bindings.
assert_fail('''
x = 1
x = 2
''', allow_static_error = True)
assert_fail('''
x = 1
x += 2
''', allow_static_error = True)


# Names after a dot are resolved dynamically.
def foo(x):
  return x.unknown()


x = []
y = x
x.append(1)
assert_eq(y, [1])


def bar(y):
  y.append(1)

w = []
bar(w)
assert_eq(x, [1])



r = 1
_ = [r for r in [2]]
assert_eq(r, 1)

