assert_eq(len({}), 0)
assert_eq(len({"key0": 0}), 1)
assert_eq(len({"key0": 0, "key1": 1}), 2)
assert_eq(len({"key0": 0, "key1": 1, "key2": 2}), 3)
assert_eq(len({"key0": 0, "key1": 1, "key2": 2, "key3": 3}), 4)

