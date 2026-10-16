## Begin module: "//:test1.bzl"
def outer():
  return [x for x in range(3)]
## End module
## Main
load("//:test1.bzl", "outer")
assert_eq(outer(), [0, 1, 2])
