a = {"key0": 0, "key1": 1}
b = {"key1": 101, "key2": 2}

# No arguments.
a.update()
assert_eq(a, {"key0": 0, "key1": 1})


# With arguments.
a.update(b)
assert_eq(a, {"key0": 0, "key1": 101, "key2": 2})

a.update([("key3", 3), ("key1", 201)])
assert_eq(a, {"key0": 0, "key1": 201, "key2": 2, "key3": 3})

a.update(key4 = 4, key5 = 5, key1 = 301)
assert_eq(a, {"key0": 0, "key1": 301, "key2": 2, "key3": 3, "key4": 4, "key5": 5})

a.update([("key8", 8), ("key9", 9)], key6 = 6, key7 = 7)
assert_eq(a, {"key0": 0, "key1": 301, "key2": 2, "key3": 3, "key4": 4, "key5": 5, "key8": 8, "key9": 9, "key6": 6, "key7": 7})

assert_fail('''{}.update([], [])''')
assert_fail('''{}.update(None)''')
assert_fail('''{}.update(1)''')
assert_fail('''{}.update([None])''')
assert_fail('''{}.update([([], None)])''')
assert_fail('''{}.update([()])''')
assert_fail('''{}.update([(1,)])''')
assert_fail('''{}.update([(1, None, None)])''')

assert_fail('''
def test():
  a = {"key0": 0, "key1": 1}
  for x in a:
    a.update()

test()
''')

