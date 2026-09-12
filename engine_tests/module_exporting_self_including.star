## Begin module: "//:test1.bzl"
a = [1, 2, 3]
a.append(a)
## End module
## Main
# This will load a structure that contains itself.
load("//:test1.bzl", "a")
print(a)

