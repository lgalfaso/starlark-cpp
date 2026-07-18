assert_fail("sorted((set(), set()))", error_message = "TypeError: '<' not supported between instances of 'set' and 'set'")
assert_fail("sorted((set(), set(), set()))", error_message = "TypeError: '<' not supported between instances of 'set' and 'set'")

assert_fail('''sorted([], None)''', error_message = 'TypeError: sorted_impl expected 1 arguments, got 2')
assert_fail('''sorted([], reverse = None)''', error_message = 'TypeError: sorted() argument reverse must be bool, not NoneType')
