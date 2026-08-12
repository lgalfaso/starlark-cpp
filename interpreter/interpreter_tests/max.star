assert_fail("max()", error_message = """Error: TypeError: max expected at least 1 argument, got 0
    1 | max()
      | ~~~^^
""")
assert_fail("max([])", error_message = """max() iterable argument is empty
    1 | max([])
      | ~~~^^^^
""")
assert_fail("max([], [], key = set)", error_message = """'<' not supported between instances of 'set' and 'set'
    1 | max([], [], key = set)
      | ~~~^^^^^^^^^^^^^^^^^^^
""")
