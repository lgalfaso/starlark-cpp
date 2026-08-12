## Begin module: "//:test1.bzl"
a = {1: 1}
## End module
## Main
# Mutate frozen value
load("//:test1.bzl", "a")
def foo():
  b = a;
  b.setdefault(1, 2)
foo()
