# A set is unhashable.
assert_fail('''
{set(): None}
''')

# A frozen set is unhashable.
assert_fail('''
## Begin module: "//:test1.bzl"
a = set()
## End module
## Main
load("//:test1.bzl", "a")
{a: None}
''')

