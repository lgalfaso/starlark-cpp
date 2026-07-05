# No arguments.
assert_fail('''{}.get()''')


# One argument.
a = {"key0": 0, "key1": 1}
assert_eq(a.get("key0"), 0)
assert_eq(a.get("key1"), 1)
assert_eq(a.get("key2"), None)

assert_fail('''{}.get({})''')
assert_fail('''{}.get([])''')


# Two arguments.
default_value = [42]
assert_eq(a.get("key0", []), 0)
assert_eq(a.get("key2", default_value), default_value)


# Test that it is possible to do a `get` while iterating.
# Context: https://github.com/bazelbuild/starlark/pull/333
def test():
  for x in a:
    print(a.get(x))

test()


# Three arguments.
assert_fail('''{}.get(None, None, None)''')


# Named arguments.
assert_fail('''{}.get(key = None)''')
assert_fail('''{}.get(None, default = None)''')

