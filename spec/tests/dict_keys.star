a = {"key0": 0, "key1": 1, "key2": 2, "key3": 3}
assert_eq(a.keys(), ["key0", "key1", "key2", "key3"])

# Calling `keys` while iterating.
def test():
  for x in a:
    print(a.keys())

test()

# One argument.
assert_fail('''{}.keys(None)''')


