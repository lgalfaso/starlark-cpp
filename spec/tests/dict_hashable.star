assert_fail('''{{}: None}''')

# Check that a frozen dict is also unhashable.
assert_fail('''
## Begin module: "//:test1.bzl"
a = {}
## End module
## Main
load("//:test1.bzl", "a")
{a : None}
''')

