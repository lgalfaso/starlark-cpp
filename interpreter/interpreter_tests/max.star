assert_fail("max()", error_message = "Error: TypeError: max expected at least 1 argument, got 0")
assert_fail("max([])", error_message = "ValueError: max() iterable argument is empty")
assert_fail("max([], [], key = set)", error_message = "TypeError: '<' not supported between instances of 'set' and 'set'")
