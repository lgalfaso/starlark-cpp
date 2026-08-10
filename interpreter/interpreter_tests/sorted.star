assert_fail("sorted((set(), set()))", error_message = """'<' not supported between instances of 'set' and 'set'
    1 | sorted((set(), set()))
      | ~~~~~~^^^^^^^^^^^^^^^^
""")
assert_fail("sorted((set(), set(), set()))", error_message = """'<' not supported between instances of 'set' and 'set'
    1 | sorted((set(), set(), set()))
      | ~~~~~~^^^^^^^^^^^^^^^^^^^^^^^
""")

assert_fail('''sorted([], None)''', error_message = """sorted expected 1 arguments, got 2
    1 | sorted([], None)
      | ~~~~~~^^^^^^^^^^
""")
assert_fail('''sorted([], reverse = None)''', error_message = """TypeError: sorted() argument reverse must be bool, not NoneType
    1 | sorted([], reverse = None)
      | ~~~~~~^^^^^^^^^^^^^^^^^^^^
""")
