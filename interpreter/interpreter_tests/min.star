assert_fail("min()", error_message = """Error: TypeError: min expected at least 1 argument, got 0
    1 | min()
      | ~~~^^
""")
assert_fail("min([])", error_message = """ValueError: min() iterable argument is empty
    1 | min([])
      | ~~~^^^^
""")
assert_fail("min([], [], key = set)", error_message = """'<' not supported between instances of 'set' and 'set'
    1 | min([], [], key = set)
      | ~~~^^^^^^^^^^^^^^^^^^^
""")
assert_eq(min(1, 2, 3), 1)
assert_eq(min(3, 2, 1), 1)
