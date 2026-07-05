a = {"key0": 0, "key1": 1}

# No arguments.
assert_eq(a.items(), [("key0", 0), ("key1", 1)])

# While iterating.
def test():
  for x in a:
    assert_eq(a.items(), [("key0", 0), ("key1", 1)])
test()

# One argument.
assert_fail('''{}.items(None)''')
