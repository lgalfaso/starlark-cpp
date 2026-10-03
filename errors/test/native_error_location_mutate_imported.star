## Begin module: "//:test1.bzl"
a = [0, 1, 2]

def foo():
  a.append(1)

## End module
## Main
load("//:test1.bzl", "foo")
foo()
