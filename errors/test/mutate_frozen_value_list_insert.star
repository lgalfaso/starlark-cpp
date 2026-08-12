## Begin module: "//:test1.bzl"
a = [0, 1]
## End module
## Main
# Mutate frozen value
load("//:test1.bzl", "a")
def foo():
  b = a;
  b.insert(2, 3)
foo()

