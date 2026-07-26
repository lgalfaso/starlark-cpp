assert_fail("""'0123456789'""", max_string_length = 9, error_message = """string must be at most 9 elements
    1 | '0123456789'
      | ^~~~~~~~~~~~
""")
assert_succeed("""'0123456789'""", max_string_length = 10)
assert_fail("""b'0123456789'""", max_string_length = 9, error_message = "TypeError: bytes must be at most 9 elements")
assert_succeed("""b'0123456789'""", max_string_length = 10)
assert_fail("""(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)""", max_sequence_size = 9, error_message = "TypeError: sequences must be at most 9 elements")
assert_succeed("""(0, 1, 2, 3, 4, 5, 6, 7, 8, 9)""", max_sequence_size = 10)
