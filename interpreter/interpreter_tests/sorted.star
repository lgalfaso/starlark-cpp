assert_fail("sorted((set(), set()))", error_message = "TypeError: '<' not supported between instances of 'set' and 'set'")
assert_fail("sorted((set(), set(), set()))", error_message = "TypeError: '<' not supported between instances of 'set' and 'set'")
