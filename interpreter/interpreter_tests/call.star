assert_eq(len([]), 0)
assert_eq(dict(), {})
assert_eq(dict(a = 1), {'a': 1})
assert_eq(list(range(*[2, 10, 3])), [2, 5, 8])
assert_eq(dict(**{'a': 1, 'b': 2}), {'a': 1, 'b': 2})
assert_fail("""
len(*1)
""", error_message = """'int' object is not iterable
    1 | len(*1)
      | ~~~^^^^
""")
assert_fail("""
dict(**1)
""", error_message = """TypeError: argument after ** must be a mapping, not int
    1 | dict(**1)
      | ~~~~^^^^^
""")
assert_fail("""
dict(**{1: 1})
""", error_message = """TypeError: keywords must be strings
    1 | dict(**{1: 1})
      | ~~~~^^^^^^^^^^
""")
assert_fail("""
dict(a = 1, **{'a': 2})
""", error_message = """TypeError: got multiple values for keyword argument 'a'
    1 | dict(a = 1, **{'a': 2})
      | ~~~~^^^^^^^^^^^^^^^^^^^
""")
assert_fail("""
dict(**['a'])
""", error_message = """TypeError: argument after ** must be a mapping, not list
    1 | dict(**['a'])
      | ~~~~^^^^^^^^^
""")
assert_fail("""
dict(**[])
""", error_message = """TypeError: argument after ** must be a mapping, not list
    1 | dict(**[])
      | ~~~~^^^^^^
""")
