assert_fail('''
## Begin module: "//:test1.bzl"
a = {None: {}}
## End module
## Main
load("//:test1.bzl", "a")
a[1] = 1
''')

