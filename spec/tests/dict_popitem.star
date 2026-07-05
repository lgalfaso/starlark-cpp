# No arguments.
a = {"key0": 0, "key1": 1, "key2": 2}
assert_eq(a.popitem(), ("key0", 0))
assert_eq(a, {"key1": 1, "key2": 2})

assert_fail('''{}.popitem()''')

# While iterating.
assert_fail('''
def test():
  a = {"key0": 0}
  for x in a:
    a.popitem()

test()
''')

# One argument.
assert_fail('''{"key0": 0}.popitem("key0")''')


# Named arguments.
assert_fail('''{"key0": 0}.popitem(key = "key0")''')

