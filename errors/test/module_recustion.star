## Begin module: "//:test1.bzl"
load("//:test2.bzl", "b")
a = [b]
## End module
## Begin module: "//:test2.bzl"
load("//:test3.bzl", "c")
b = [c]
## End module
## Begin module: "//:test3.bzl"
load("//:test1.bzl", "a")
c = [a]
## End module
## Main
load("//:test1.bzl", "a")


