## Begin module: "//:test1.bzl"
def fn1():
  pass
def outer():
  def inner():
    return fn1
  return inner
## End module
## Begin module: "//:test2.bzl"
load("//:test1.bzl", "outer")
fn2 = outer()
fn3 = fn2()
## End module
## Main
load("//:test1.bzl", "outer")
load("//:test2.bzl", "fn3", "fn2")

assert_true(outer() != fn2)
assert_true(outer()() == fn2())
assert_true(outer()() == fn3)
