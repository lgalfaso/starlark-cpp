## Begin module: "//:test1.bzl"
a = set([1])
## End module
## Main
# Mutate frozen value
load("//:test1.bzl", "a")
def foo():
  b = a;
  b.difference_update(set([2]))
foo()

