
assert_eq(str({}), "{}")
assert_eq(str({'key1': 1}), '{"key1": 1}')
assert_eq(str({'key1': 1, 'key2': 2}), '{"key1": 1, "key2": 2}')

a = {"key1": 1, "key2": 2}
a["key3"] = a
assert_eq(str(a), '{"key1": 1, "key2": 2, "key3": {...}}')

