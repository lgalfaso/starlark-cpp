
# No arguments.
assert_fail('''
a = {"key0": 0, "key1": 1, "key2": 2}
a.pop()
''')


# One argument.
a = {"key0": 0, "key1": 1, "key2": 2}

assert_eq(a.pop("key0"), 0)
assert_eq(a, {"key1": 1, "key2": 2})
assert_eq(a.pop("key1"), 1)
assert_eq(a, {"key2": 2})

assert_fail('''
a = {"key0": 0, "key1": 1, "key2": 2}
a.pop("key3")
''')


# Two arguments.
b = {"key0": 0, "key1": 1, "key2": 2}
default_value = [42]
assert_eq(b.pop("key0", default_value), 0)
assert_eq(b, {"key1": 1, "key2": 2})
assert_eq(b.pop("key3", default_value), default_value)
assert_eq(b, {"key1": 1, "key2": 2})

# Unhashable key.
assert_fail('''
a = {"key0": 0, "key1": 1, "key2": 2}
a.pop([], 42)
''')

# While iterating.
assert_fail('''
a = {"key0": 0, "key1": 1, "key2": 2}

def test():
  for x in a:
    a.pop(x)

test()
''')


# Three arguments.
assert_fail('''{}.pop(0, 0, None)''')


# Named arguments.
assert_fail('''{"key": 0}.pop(key = "key")''')
assert_fail('''{"key": 0}.pop("key", default = [42])''')

