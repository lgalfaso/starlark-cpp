## Begin module: "//:test1.bzl"
a = set([1])
## End module
## Main
load("//:test1.bzl", "b")
print(b)

