## Begin module: "//:test1.bzl"
a = [].append
## End module
## Begin module: "//:test2.bzl"
load("//:test1.bzl", "a")
b = a
## End module
## Main
load("//:test1.bzl", "a")
load("//:test2.bzl", "b")

assert_eq(a, b)
