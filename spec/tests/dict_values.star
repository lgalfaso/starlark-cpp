a = {"key0": 0, "key1": 1, "key2": 2}

# No arguments.
assert_eq(a.values(), [0, 1, 2])


# One argument.
assert_fail('''{}.values(None)''')


# Named argument.
assert_fail('''{}.values(key = None)''')


# While iterating
def test():
  for x in a:
    print(a.values())

test()

