## Begin module: "//:test1.bzl"
def outer():
  return {k: k for k in range(2)}
## End module
## Main
load("//:test1.bzl", "outer")
assert_eq(outer(), {0: 0, 1: 1})
