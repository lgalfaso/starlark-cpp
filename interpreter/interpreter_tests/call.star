assert_eq(len([]), 0)
assert_eq(dict(), {})
assert_eq(dict(a = 1), {'a': 1})
assert_eq(list(range(*[2, 10, 3])), [2, 5, 8])
assert_eq(dict(**{'a': 1, 'b': 2}), {'a': 1, 'b': 2})
assert_fail("""
len(*1)
""", error_message = "TypeError: 'int' object is not iterable")
assert_fail("""
dict(**1)
""", error_message = "TypeError: argument after ** must be a mapping, not int")
assert_fail("""
dict(**{1: 1})
""", error_message = "TypeError: keywords must be strings")
assert_fail("""
dict(a = 1, **{'a': 2})
""", error_message = "TypeError: got multiple values for keyword argument 'a'")
assert_fail("""
dict(**['a'])
""", error_message = "TypeError: argument after ** must be a mapping, not list")
assert_fail("""
dict(**[])
""", error_message = "TypeError: argument after ** must be a mapping, not list")
