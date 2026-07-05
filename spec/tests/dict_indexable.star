a = {"key0": 0, "key1": 1}
assert_eq(a["key0"], 0)
assert_eq(a["key1"], 1)

assert_fail('''{}["key0"]''')

a["key2"] = 2
assert_eq(a["key2"], 2)

assert_fail('''{}[[]] = 1''')


assert_fail('''
def test():
  a = {"key0": 0, "key1": 1}
  for x in a:
    a[x] = None

test()
''')

assert_fail('''
## Begin module: "//:test1.bzl"
a = {"key0": 0, "key1": 1}
## End module
## Main
load("//:test1.bzl", "a")
a["key2"] = None
''')

