## Begin module: "//:test1.bzl"
def fn1():
  pass
## End module
## Begin module: "//:test2.bzl"
load("//:test1.bzl", fnX = "fn1")
fn2 = fnX
def fn1():
  pass
## End module
## Main
load("//:test1.bzl", "fn1")
load("//:test2.bzl", "fn2", fn3 = "fn1")

assert_true(fn1 == fn2)
assert_true(fn1 != fn3)
