# No arguments.
assert_fail('''{"key0": 0}.setdefault()''')


# One argument.
a = {"key0": 0, "key1": 1}
assert_eq(a.setdefault("key1"), 1)
assert_eq(a.setdefault("key2"), None)
assert_eq(a, {"key0": 0, "key1": 1, "key2": None})


# Two arguments.
default_value = [42]
assert_eq(a.setdefault("key1", default_value), 1)
assert_eq(a, {"key0": 0, "key1": 1, "key2": None})
assert_eq(a.setdefault("key3", default_value), default_value)
assert_eq(a, {"key0": 0, "key1": 1, "key2": None, "key3": default_value})

assert_fail('''{}.setdefault([])''')


# Three arguments.
assert_fail('''{}.setdefault(None, None, None)''')


# Named arguments.
assert_fail('''{}.setdefault(key = "key")''')
assert_fail('''{}.setdefault("key", default = None)''')


# While iterating.
assert_fail('''
a = {"key0": 0, "key1": 1}
def test():
  for x in a:
    a.setdefault(None)

test()
''')


