assert_fail('''
## Begin module: "//:test1.bzl"
a = 1
## End module
## Main
load("//:test1.bzl", "bb")
''')

assert_fail('''
## Begin module: "//:test1.bzl"
a = 1
## End module
## Main
load("//:test1.bzl")
''', allow_static_error = True)

assert_succeed('''
## Begin module: "//:test1.bzl"
a = 1
## End module
## Main
load("//:test1.bzl", x = "a")
def test():
  if x != 1: fail("Expect x to equals 1")
test()
''')

assert_fail('''
## Begin module: "//:test1.bzl"
a = 1
## End module
## Main
load("//:test1.bzl", x = "a")
print(a)
''', allow_static_error = True)

assert_fail('''
## Begin module: "//:test1.bzl"
_a = 1
## End module
## Main
load("//:test1.bzl", x = "_a")
''', allow_static_error = True)

assert_succeed('''
## Begin module: "//:test1.bzl"
a = 1
## End module
## Main
load("//:test1.bzl", x = "a", y = "a")
''')
